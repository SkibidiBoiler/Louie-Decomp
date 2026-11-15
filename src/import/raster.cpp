#include "import/raster.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <atomic>
#include <mutex>
#include <thread>
#include <cmath>
#include <cstring>
#include <unordered_map>

#include "plutovg.h"
#include "stb_image.h"

namespace swfimport {

namespace {

// ---------------------------------------------------------------------------------------------------------------
// Geometry helpers

struct Xform { // x' = a x + c y + tx, y' = b x + d y + ty
    float a = 1, b = 0, c = 0, d = 1, tx = 0, ty = 0;
    void apply(float x, float y, float &ox, float &oy) const
    {
        ox = a * x + c * y + tx;
        oy = b * x + d * y + ty;
    }
    Xform operator*(const Xform &m) const // this ∘ m
    {
        return {a * m.a + c * m.b, b * m.a + d * m.b, a * m.c + c * m.d, b * m.c + d * m.d,
                a * m.tx + c * m.ty + tx, b * m.tx + d * m.ty + ty};
    }
    float scale() const { return std::sqrt(std::fabs(a * d - b * c)); }
};

Xform fromMatrix(const Matrix &m) { return {m.a, m.b, m.c, m.d, m.tx, m.ty}; }

plutovg_matrix_t toPvg(const Xform &x) { return {x.a, x.b, x.c, x.d, x.tx, x.ty}; }

struct Color {
    float r = 0, g = 0, b = 0, a = 1;
};

struct ColorX { // colour transform
    float mult[4] = {1, 1, 1, 1};
    float add[4] = {0, 0, 0, 0};
    Color apply(const Color &c) const
    {
        auto f = [](float v) { return v < 0 ? 0.0f : (v > 1 ? 1.0f : v); };
        return {f(c.r * mult[0] + add[0]), f(c.g * mult[1] + add[1]), f(c.b * mult[2] + add[2]),
                f(c.a * mult[3] + add[3])};
    }
    ColorX operator*(const ColorX &child) const
    {
        ColorX out;
        for (int i = 0; i < 4; ++i) {
            out.mult[i] = mult[i] * child.mult[i];
            out.add[i] = mult[i] * child.add[i] + add[i];
        }
        return out;
    }
};

ColorX fromCx(const CxForm &cx)
{
    ColorX out;
    std::memcpy(out.mult, cx.mult, sizeof(out.mult));
    std::memcpy(out.add, cx.add, sizeof(out.add));
    return out;
}

// ---------------------------------------------------------------------------------------------------------------
// Shapes

struct Gradient {
    std::vector<std::pair<float, Color>> stops;
    uint8_t spread = 0;
    float focal = 0;
};

struct FillStyle {
    uint8_t type = 0; // 0x00 solid, 0x10 linear, 0x12 radial, 0x13 focal, 0x40-0x43 bitmap
    Color color;
    Matrix matrix;
    Gradient gradient;
    uint16_t bitmapId = 0;
};

struct LineStyle {
    float width = 0; // pixels
    Color color;
    uint8_t startCap = 0, endCap = 0, join = 0; // 0 round, 1 none/bevel, 2 square/miter
    float miter = 3;
    bool hasFill = false;
    FillStyle fill;
};

struct Edge { // twips
    int32_t x0, y0, cx, cy, x1, y1;
    bool curve;
};

struct Layer {
    std::vector<FillStyle> fills;
    std::vector<LineStyle> lines;
    std::vector<std::vector<Edge>> fillEdges;               // per fill style
    std::vector<std::vector<std::vector<Edge>>> linePaths;  // per line style: connected runs
};

struct Shape {
    Rect bounds;
    std::vector<Layer> layers;
};

Color readRgb(const uint8_t *p, bool alpha)
{
    return {p[0] / 255.0f, p[1] / 255.0f, p[2] / 255.0f, alpha ? p[3] / 255.0f : 1.0f};
}

class ShapeParser {
public:
    ShapeParser(const Tag &tag) : t_(tag) {}

    bool parse(Shape &out)
    {
        version_ = t_.code == 2 ? 1 : t_.code == 22 ? 2 : t_.code == 32 ? 3 : 4;
        size_t p = 2;
        out.bounds = readRect(t_.body, t_.size, p);
        if (version_ == 4) {
            readRect(t_.body, t_.size, p); // edge bounds
            ++p;                           // flags
        }
        out.layers.emplace_back();
        Layer *layer = &out.layers.back();
        if (!readStyles(p, *layer)) {
            return false;
        }
        Bits b(t_.body, t_.size, p);
        int fillBits = static_cast<int>(b.ub(4));
        int lineBits = static_cast<int>(b.ub(4));
        int32_t x = 0, y = 0;
        uint32_t fill0 = 0, fill1 = 0, line = 0;
        std::vector<Edge> *run = nullptr;
        while (b.pos() < t_.size) {
            if (b.ub(1) == 0) { // style change / end
                const uint32_t flags = b.ub(5);
                if (flags == 0) {
                    break;
                }
                if (flags & 0x01) {
                    const int n = static_cast<int>(b.ub(5));
                    x = b.sb(n);
                    y = b.sb(n);
                    run = nullptr;
                }
                if (flags & 0x02) {
                    fill0 = b.ub(fillBits);
                }
                if (flags & 0x04) {
                    fill1 = b.ub(fillBits);
                }
                if (flags & 0x08) {
                    line = b.ub(lineBits);
                    run = nullptr;
                }
                if (flags & 0x10) { // new styles: a new layer drawn above the previous ones
                    size_t q = b.align();
                    out.layers.emplace_back();
                    layer = &out.layers.back();
                    if (!readStyles(q, *layer)) {
                        return false;
                    }
                    b = Bits(t_.body, t_.size, q);
                    fillBits = static_cast<int>(b.ub(4));
                    lineBits = static_cast<int>(b.ub(4));
                    run = nullptr;
                }
                continue;
            }
            Edge e{};
            e.x0 = x;
            e.y0 = y;
            const bool straight = b.ub(1) != 0;
            const int n = static_cast<int>(b.ub(4)) + 2;
            if (straight) {
                int32_t dx = 0, dy = 0;
                if (b.ub(1)) {
                    dx = b.sb(n);
                    dy = b.sb(n);
                } else if (b.ub(1)) {
                    dy = b.sb(n);
                } else {
                    dx = b.sb(n);
                }
                e.x1 = x + dx;
                e.y1 = y + dy;
                e.cx = (e.x0 + e.x1) / 2;
                e.cy = (e.y0 + e.y1) / 2;
                e.curve = false;
            } else {
                const int32_t cdx = b.sb(n), cdy = b.sb(n), adx = b.sb(n), ady = b.sb(n);
                e.cx = x + cdx;
                e.cy = y + cdy;
                e.x1 = e.cx + adx;
                e.y1 = e.cy + ady;
                e.curve = true;
            }
            x = e.x1;
            y = e.y1;
            if (fill1 && fill1 <= layer->fills.size()) {
                layer->fillEdges[fill1 - 1].push_back(e);
            }
            if (fill0 && fill0 <= layer->fills.size()) {
                layer->fillEdges[fill0 - 1].push_back(Edge{e.x1, e.y1, e.cx, e.cy, e.x0, e.y0, e.curve});
            }
            if (line && line <= layer->lines.size()) {
                if (!run) {
                    layer->linePaths[line - 1].emplace_back();
                    run = &layer->linePaths[line - 1].back();
                }
                run->push_back(e);
            }
        }
        return true;
    }

private:
    bool readFill(size_t &p, FillStyle &f)
    {
        const uint8_t *d = t_.body;
        f.type = d[p++];
        if (f.type == 0x00) {
            f.color = readRgb(d + p, version_ >= 3);
            p += version_ >= 3 ? 4 : 3;
        } else if (f.type == 0x10 || f.type == 0x12 || f.type == 0x13) {
            f.matrix = readMatrix(d, t_.size, p);
            const uint8_t flags = d[p++];
            f.gradient.spread = static_cast<uint8_t>(flags >> 6);
            const int count = flags & 0x0F;
            for (int i = 0; i < count; ++i) {
                const float ratio = d[p] / 255.0f;
                const Color c = readRgb(d + p + 1, version_ >= 3);
                p += 1 + (version_ >= 3 ? 4 : 3);
                f.gradient.stops.emplace_back(ratio, c);
            }
            if (f.type == 0x13) {
                f.gradient.focal = static_cast<float>(static_cast<int16_t>(u16(d + p))) / 256.0f;
                p += 2;
            }
        } else if (f.type >= 0x40 && f.type <= 0x43) {
            f.bitmapId = u16(d + p);
            p += 2;
            f.matrix = readMatrix(d, t_.size, p);
        } else {
            return false;
        }
        return p <= t_.size;
    }

    bool readStyles(size_t &p, Layer &layer)
    {
        const uint8_t *d = t_.body;
        size_t count = d[p++];
        if (count == 0xFF && version_ >= 2) {
            count = u16(d + p);
            p += 2;
        }
        layer.fills.resize(count);
        layer.fillEdges.resize(count);
        for (size_t i = 0; i < count; ++i) {
            if (!readFill(p, layer.fills[i])) {
                return false;
            }
        }
        count = d[p++];
        if (count == 0xFF) {
            count = u16(d + p);
            p += 2;
        }
        layer.lines.resize(count);
        layer.linePaths.resize(count);
        for (size_t i = 0; i < count; ++i) {
            LineStyle &l = layer.lines[i];
            l.width = static_cast<float>(u16(d + p) / 20.0);
            p += 2;
            if (version_ == 4) {
                const uint8_t f1 = d[p], f2 = d[p + 1];
                p += 2;
                l.startCap = static_cast<uint8_t>(f1 >> 6);
                l.join = static_cast<uint8_t>((f1 >> 4) & 3);
                l.hasFill = (f1 & 0x08) != 0;
                l.endCap = static_cast<uint8_t>(f2 & 3);
                if (l.join == 2) {
                    l.miter = static_cast<float>(u16(d + p) / 256.0);
                    p += 2;
                }
                if (l.hasFill) {
                    if (!readFill(p, l.fill)) {
                        return false;
                    }
                } else {
                    l.color = readRgb(d + p, true);
                    p += 4;
                }
            } else {
                l.color = readRgb(d + p, version_ >= 3);
                p += version_ >= 3 ? 4 : 3;
            }
        }
        return p <= t_.size;
    }

    const Tag &t_;
    int version_ = 1;
};

// Joins the edges of one fill style into closed contours (fill1 edges as stored, fill0 edges reversed).
std::vector<std::vector<Edge>> contours(const std::vector<Edge> &edges)
{
    std::unordered_multimap<uint64_t, size_t> byStart;
    auto key = [](int32_t x, int32_t y) {
        return (static_cast<uint64_t>(static_cast<uint32_t>(x)) << 32) | static_cast<uint32_t>(y);
    };
    for (size_t i = 0; i < edges.size(); ++i) {
        byStart.emplace(key(edges[i].x0, edges[i].y0), i);
    }
    std::vector<bool> used(edges.size(), false);
    std::vector<std::vector<Edge>> out;
    for (size_t i = 0; i < edges.size(); ++i) {
        if (used[i]) {
            continue;
        }
        std::vector<Edge> loop;
        size_t cur = i;
        while (true) {
            used[cur] = true;
            loop.push_back(edges[cur]);
            const Edge &e = edges[cur];
            if (e.x1 == loop.front().x0 && e.y1 == loop.front().y0) {
                break;
            }
            size_t next = SIZE_MAX;
            auto range = byStart.equal_range(key(e.x1, e.y1));
            for (auto it = range.first; it != range.second; ++it) {
                if (!used[it->second]) {
                    next = it->second;
                    break;
                }
            }
            if (next == SIZE_MAX) {
                break;
            }
            cur = next;
        }
        out.push_back(std::move(loop));
    }
    return out;
}

} // namespace

// ---------------------------------------------------------------------------------------------------------------

struct Rasterizer::Impl {
    const Swf &swf;
    const SwfIndex &index;
    std::vector<uint8_t> jpegTables;
    std::unordered_map<uint16_t, Shape> shapes;
    std::unordered_map<uint16_t, plutovg_surface_t *> bitmaps;
    std::unordered_map<uint16_t, TimelineData> timelines;

    Impl(const Swf &s, const SwfIndex &i) : swf(s), index(i)
    {
        const auto &d = swf.data();
        forEachTag(d.data(), swf.firstTag(), d.size(), [&](const Tag &tag) {
            if (tag.code == 8 && jpegTables.empty()) {
                jpegTables.assign(tag.body, tag.body + tag.size);
            }
        });
    }

    ~Impl()
    {
        for (auto &b : bitmaps) {
            if (b.second) {
                plutovg_surface_destroy(b.second);
            }
        }
    }

    const Shape *shape(uint16_t id)
    {
        auto it = shapes.find(id);
        if (it != shapes.end()) {
            return &it->second;
        }
        auto def = index.defs.find(id);
        Shape s;
        if (def == index.defs.end() || !ShapeParser(def->second).parse(s)) {
            SDL_Log("Import: shape %u could not be read", id);
            s.layers.clear();
        }
        return &shapes.emplace(id, std::move(s)).first->second;
    }

    const TimelineData *timeline(uint16_t id)
    {
        auto it = timelines.find(id);
        if (it != timelines.end()) {
            return &it->second;
        }
        auto def = index.defs.find(id);
        if (def == index.defs.end() || def->second.size < 4) {
            return nullptr;
        }
        const Tag &tag = def->second;
        return &timelines.emplace(id, parseTimeline(tag.body, 4, tag.size, u16(tag.body + 2))).first->second;
    }

    // Decodes a bitmap character into a premultiplied ARGB surface (PlutoVG's format).
    std::mutex bitmapMutex; // shapes are drawn on several threads; they share decoded bitmaps

    plutovg_surface_t *bitmap(uint16_t id)
    {
        std::lock_guard<std::mutex> lock(bitmapMutex);
        auto it = bitmaps.find(id);
        if (it != bitmaps.end()) {
            return it->second;
        }
        plutovg_surface_t *surface = decodeBitmap(id);
        bitmaps[id] = surface;
        return surface;
    }

    plutovg_surface_t *decodeBitmap(uint16_t id)
    {
        auto def = index.defs.find(id);
        if (def == index.defs.end()) {
            return nullptr;
        }
        const Tag &tag = def->second;
        const uint8_t *b = tag.body;
        if (tag.code == 20 || tag.code == 36) {
            return decodeLossless(tag);
        }
        std::vector<uint8_t> image;
        const uint8_t *alpha = nullptr;
        size_t alphaSize = 0;
        if (tag.code == 6 && jpegTables.size() <= 4) {
            image.assign(b + 2, b + tag.size); // empty JPEGTables: the data is a complete JPEG
        } else if (tag.code == 6) {
            // JPEGTables + this data, without the end/start markers in between.
            image = jpegTables;
            while (image.size() >= 2 && image[image.size() - 2] == 0xFF && image[image.size() - 1] == 0xD9) {
                image.resize(image.size() - 2);
            }
            size_t start = 2;
            if (tag.size > 6 && b[2] == 0xFF && b[3] == 0xD9 && b[4] == 0xFF && b[5] == 0xD8) {
                start = 6;
            }
            image.insert(image.end(), b + start + 2, b + tag.size);
        } else if (tag.code == 21) {
            image.assign(b + 2, b + tag.size);
        } else if (tag.code == 35 || tag.code == 90) {
            const uint32_t offset = u32(b + 2);
            const size_t start = tag.code == 35 ? 6 : 8;
            image.assign(b + start, b + std::min<size_t>(start + offset, tag.size));
            alpha = b + start + offset;
            alphaSize = tag.size > start + offset ? tag.size - start - offset : 0;
        } else {
            return nullptr;
        }
        // Old encoders put a stray EOI/SOI pair in front of the data.
        if (image.size() > 4 && image[0] == 0xFF && image[1] == 0xD9 && image[2] == 0xFF && image[3] == 0xD8) {
            image.erase(image.begin(), image.begin() + 4);
        }
        int w = 0, h = 0, comp = 0;
        unsigned char *pixels = stbi_load_from_memory(image.data(), static_cast<int>(image.size()), &w, &h, &comp, 4);
        if (!pixels) {
            SDL_Log("Import: bitmap %u could not be decoded (%s)", id, stbi_failure_reason());
            return nullptr;
        }
        std::vector<uint8_t> alphaBytes;
        if (alpha && alphaSize) {
            alphaBytes = inflate(alpha, alphaSize, static_cast<size_t>(w) * h);
        }
        plutovg_surface_t *s = plutovg_surface_create(w, h);
        auto *dst = plutovg_surface_get_data(s);
        const int stride = plutovg_surface_get_stride(s);
        for (int y = 0; y < h; ++y) {
            auto *row = reinterpret_cast<uint32_t *>(dst + static_cast<size_t>(y) * stride);
            for (int x = 0; x < w; ++x) {
                const unsigned char *px = pixels + (static_cast<size_t>(y) * w + x) * 4;
                uint32_t a = px[3];
                if (alphaBytes.size() == static_cast<size_t>(w) * h) {
                    a = alphaBytes[static_cast<size_t>(y) * w + x];
                }
                // JPEG3 colour data is already premultiplied by its alpha; others are straight.
                const bool premul = alpha != nullptr;
                const uint32_t r = premul ? std::min<uint32_t>(px[0], a) : px[0] * a / 255;
                const uint32_t g = premul ? std::min<uint32_t>(px[1], a) : px[1] * a / 255;
                const uint32_t bl = premul ? std::min<uint32_t>(px[2], a) : px[2] * a / 255;
                row[x] = (a << 24) | (r << 16) | (g << 8) | bl;
            }
        }
        stbi_image_free(pixels);
        return s;
    }

    plutovg_surface_t *decodeLossless(const Tag &tag)
    {
        const uint8_t *b = tag.body;
        const bool alpha = tag.code == 36;
        const uint8_t format = b[2];
        const int w = u16(b + 3), h = u16(b + 5);
        if (w <= 0 || h <= 0) {
            return nullptr;
        }
        plutovg_surface_t *s = plutovg_surface_create(w, h);
        auto *dst = plutovg_surface_get_data(s);
        const int stride = plutovg_surface_get_stride(s);
        auto put = [&](int x, int y, uint32_t a, uint32_t r, uint32_t g, uint32_t bl, bool premultiplied) {
            if (!premultiplied) {
                r = r * a / 255;
                g = g * a / 255;
                bl = bl * a / 255;
            }
            reinterpret_cast<uint32_t *>(dst + static_cast<size_t>(y) * stride)[x] = (a << 24) | (r << 16) | (g << 8) | bl;
        };
        if (format == 3) {
            const size_t count = static_cast<size_t>(b[7]) + 1;
            const std::vector<uint8_t> raw = inflate(b + 8, tag.size - 8);
            const size_t step = alpha ? 4 : 3;
            const size_t rowBytes = (static_cast<size_t>(w) + 3) & ~static_cast<size_t>(3);
            if (raw.size() < step * count + rowBytes * h) {
                plutovg_surface_destroy(s);
                return nullptr;
            }
            const uint8_t *data = raw.data() + step * count;
            for (int y = 0; y < h; ++y) {
                for (int x = 0; x < w; ++x) {
                    const size_t i = data[static_cast<size_t>(y) * rowBytes + x];
                    if (i >= count) {
                        put(x, y, 0, 0, 0, 0, true);
                        continue;
                    }
                    const uint8_t *c = &raw[i * step];
                    put(x, y, alpha ? c[3] : 255, c[0], c[1], c[2], alpha); // Lossless2 palettes are premultiplied
                }
            }
        } else if (format == 4 || format == 5) {
            const std::vector<uint8_t> raw = inflate(b + 7, tag.size - 7);
            const size_t rowBytes = format == 4 ? ((static_cast<size_t>(w) * 2 + 3) & ~static_cast<size_t>(3))
                                                : static_cast<size_t>(w) * 4;
            if (raw.size() < rowBytes * h) {
                plutovg_surface_destroy(s);
                return nullptr;
            }
            for (int y = 0; y < h; ++y) {
                const uint8_t *row = raw.data() + static_cast<size_t>(y) * rowBytes;
                for (int x = 0; x < w; ++x) {
                    if (format == 4) {
                        const uint16_t v = static_cast<uint16_t>((row[2 * x] << 8) | row[2 * x + 1]);
                        put(x, y, 255, ((v >> 10) & 31) * 255 / 31, ((v >> 5) & 31) * 255 / 31, (v & 31) * 255 / 31, true);
                    } else {
                        const uint8_t *px = row + 4 * x;
                        put(x, y, alpha ? px[0] : 255, px[1], px[2], px[3], true); // ARGB, premultiplied in Lossless2
                    }
                }
            }
        } else {
            plutovg_surface_destroy(s);
            return nullptr;
        }
        return s;
    }

    // --- drawing -------------------------------------------------------------------------------------------

    static void addEdges(plutovg_canvas_t *cv, const std::vector<Edge> &run, const Xform &m, bool close)
    {
        float x = 0, y = 0;
        m.apply(static_cast<float>(run.front().x0 / 20.0), static_cast<float>(run.front().y0 / 20.0), x, y);
        plutovg_canvas_move_to(cv, x, y);
        for (const Edge &e : run) {
            float ex = 0, ey = 0;
            m.apply(static_cast<float>(e.x1 / 20.0), static_cast<float>(e.y1 / 20.0), ex, ey);
            if (e.curve) {
                float cx = 0, cy = 0;
                m.apply(static_cast<float>(e.cx / 20.0), static_cast<float>(e.cy / 20.0), cx, cy);
                plutovg_canvas_quad_to(cv, cx, cy, ex, ey);
            } else {
                plutovg_canvas_line_to(cv, ex, ey);
            }
        }
        if (close) {
            plutovg_canvas_close_path(cv);
        }
    }

    // Sets the canvas paint for a fill style in device space.
    bool setPaint(plutovg_canvas_t *cv, const FillStyle &f, const Xform &m, const ColorX &cx)
    {
        if (f.type == 0x00) {
            const Color c = cx.apply(f.color);
            plutovg_canvas_set_rgba(cv, c.r, c.g, c.b, c.a);
            return true;
        }
        if (f.type == 0x10 || f.type == 0x12 || f.type == 0x13) {
            std::vector<plutovg_gradient_stop_t> stops;
            for (const auto &[ratio, color] : f.gradient.stops) {
                const Color c = cx.apply(color);
                stops.push_back({ratio, {c.r, c.g, c.b, c.a}});
            }
            if (stops.empty()) {
                return false;
            }
            const plutovg_spread_method_t spread = f.gradient.spread == 1   ? PLUTOVG_SPREAD_METHOD_REFLECT
                                                   : f.gradient.spread == 2 ? PLUTOVG_SPREAD_METHOD_REPEAT
                                                                            : PLUTOVG_SPREAD_METHOD_PAD;
            // The gradient square is -16384..16384 twips = -819.2..819.2 px before the gradient matrix.
            const plutovg_matrix_t pm = toPvg(m * fromMatrix(f.matrix));
            constexpr float kR = 819.2f;
            if (f.type == 0x10) {
                plutovg_canvas_set_linear_gradient(cv, -kR, 0, kR, 0, spread, stops.data(), static_cast<int>(stops.size()), &pm);
            } else {
                plutovg_canvas_set_radial_gradient(cv, 0, 0, kR, f.gradient.focal * kR, 0, 0, spread, stops.data(),
                                                   static_cast<int>(stops.size()), &pm);
            }
            return true;
        }
        if (f.type >= 0x40 && f.type <= 0x43) {
            plutovg_surface_t *bmp = bitmap(f.bitmapId);
            if (!bmp) {
                plutovg_canvas_set_rgba(cv, 1, 0, 0, 1); // Flash shows missing bitmaps as red
                return true;
            }
            Xform bm = fromMatrix(f.matrix); // bitmap pixels -> twips scale
            bm.a /= 20.0f;
            bm.b /= 20.0f;
            bm.c /= 20.0f;
            bm.d /= 20.0f;
            const plutovg_matrix_t pm = toPvg(m * bm);
            const bool repeat = f.type == 0x40 || f.type == 0x42;
            plutovg_canvas_set_texture(cv, bmp, repeat ? PLUTOVG_TEXTURE_TYPE_TILED : PLUTOVG_TEXTURE_TYPE_PLAIN,
                                       std::clamp(cx.mult[3] + cx.add[3], 0.0f, 1.0f), &pm);
            return true;
        }
        return false;
    }

    void drawShape(plutovg_canvas_t *cv, const Shape &s, const Xform &m, const ColorX &cx)
    {
        plutovg_canvas_set_fill_rule(cv, PLUTOVG_FILL_RULE_EVEN_ODD);
        for (const Layer &layer : s.layers) {
            for (size_t i = 0; i < layer.fills.size(); ++i) {
                if (layer.fillEdges[i].empty()) {
                    continue;
                }
                plutovg_canvas_new_path(cv);
                for (const auto &loop : contours(layer.fillEdges[i])) {
                    addEdges(cv, loop, m, true);
                }
                if (setPaint(cv, layer.fills[i], m, cx)) {
                    plutovg_canvas_fill(cv);
                }
            }
            for (size_t i = 0; i < layer.lines.size(); ++i) {
                const LineStyle &l = layer.lines[i];
                if (layer.linePaths[i].empty()) {
                    continue;
                }
                plutovg_canvas_new_path(cv);
                for (const auto &run : layer.linePaths[i]) {
                    const bool closed = run.front().x0 == run.back().x1 && run.front().y0 == run.back().y1;
                    addEdges(cv, run, m, closed);
                }
                // Flash never draws a stroke thinner than one screen pixel (hairlines).
                plutovg_canvas_set_line_width(cv, std::max(l.width * m.scale(), 1.0f));
                plutovg_canvas_set_line_cap(cv, l.startCap == 1   ? PLUTOVG_LINE_CAP_BUTT
                                                : l.startCap == 2 ? PLUTOVG_LINE_CAP_SQUARE
                                                                  : PLUTOVG_LINE_CAP_ROUND);
                plutovg_canvas_set_line_join(cv, l.join == 1   ? PLUTOVG_LINE_JOIN_BEVEL
                                                 : l.join == 2 ? PLUTOVG_LINE_JOIN_MITER
                                                               : PLUTOVG_LINE_JOIN_ROUND);
                plutovg_canvas_set_miter_limit(cv, l.miter);
                bool ok = true;
                if (l.hasFill) {
                    ok = setPaint(cv, l.fill, m, cx);
                } else {
                    const Color c = cx.apply(l.color);
                    plutovg_canvas_set_rgba(cv, c.r, c.g, c.b, c.a);
                }
                if (ok) {
                    plutovg_canvas_stroke(cv);
                }
            }
        }
        plutovg_canvas_new_path(cv);
    }

    // Adds the fill geometry of a character (shape, or sprite frame 0 recursively) to the current path.
    void addMaskGeometry(plutovg_canvas_t *cv, uint16_t id, const Xform &m, int depth)
    {
        auto kind = index.chars.find(id);
        if (kind == index.chars.end() || depth > 32) {
            return;
        }
        if (kind->second == KindShape) {
            for (const Layer &layer : shape(id)->layers) {
                for (const auto &edges : layer.fillEdges) {
                    if (!edges.empty()) {
                        for (const auto &loop : contours(edges)) {
                            addEdges(cv, loop, m, true);
                        }
                    }
                }
            }
        } else if (kind->second == KindSprite) {
            if (const TimelineData *tl = timeline(id)) {
                for (const auto &t : tl->tracks) {
                    if (t->first <= 0 && t->last >= 0 && t->clipDepth == 0) {
                        addMaskGeometry(cv, t->charId, m * fromMatrix(keyAt(*t, 0).matrix), depth + 1);
                    }
                }
            }
        }
    }

    static const Key &keyAt(const TrackData &t, int frame)
    {
        const Key *best = &t.keys.front();
        for (const Key &k : t.keys) {
            if (k.frame <= frame) {
                best = &k;
            }
        }
        return *best;
    }

    // Draws frame `frame` of a sprite; nested sprites show their first frame (as freshly placed instances).
    void drawCharacter(plutovg_canvas_t *cv, uint16_t id, int frame, const Xform &m, const ColorX &cx, int depth)
    {
        auto kind = index.chars.find(id);
        if (kind == index.chars.end() || depth > 32) {
            return;
        }
        if (kind->second == KindShape) {
            drawShape(cv, *shape(id), m, cx);
            return;
        }
        if (kind->second == KindBitmap) {
            if (plutovg_surface_t *bmp = bitmap(id)) {
                const plutovg_matrix_t pm = toPvg(m);
                plutovg_canvas_set_texture(cv, bmp, PLUTOVG_TEXTURE_TYPE_PLAIN, std::clamp(cx.mult[3], 0.0f, 1.0f), &pm);
                const float w = static_cast<float>(plutovg_surface_get_width(bmp));
                const float h = static_cast<float>(plutovg_surface_get_height(bmp));
                const float corners[4][2] = {{0, 0}, {w, 0}, {w, h}, {0, h}};
                plutovg_canvas_new_path(cv);
                for (int i = 0; i < 4; ++i) {
                    float x = 0, y = 0;
                    m.apply(corners[i][0], corners[i][1], x, y);
                    i == 0 ? plutovg_canvas_move_to(cv, x, y) : plutovg_canvas_line_to(cv, x, y);
                }
                plutovg_canvas_close_path(cv);
                plutovg_canvas_fill(cv);
            }
            return;
        }
        if (kind->second != KindSprite) {
            return; // text, buttons and morphs are not part of the sheets
        }
        const TimelineData *tl = timeline(id);
        if (!tl) {
            return;
        }
        std::vector<const TrackData *> alive;
        for (const auto &t : tl->tracks) {
            if (frame >= t->first && frame <= t->last) {
                alive.push_back(t.get());
            }
        }
        std::stable_sort(alive.begin(), alive.end(), [](auto *a, auto *b) { return a->depth < b->depth; });
        std::vector<uint16_t> maskEnds; // clip depth of each active mask (canvas saved once per mask)
        for (const TrackData *t : alive) {
            while (!maskEnds.empty() && t->depth > maskEnds.back()) {
                plutovg_canvas_restore(cv);
                maskEnds.pop_back();
            }
            const Key &key = keyAt(*t, frame);
            const Xform child = m * fromMatrix(key.matrix);
            if (t->clipDepth > 0) {
                plutovg_canvas_save(cv);
                plutovg_canvas_new_path(cv);
                plutovg_canvas_set_fill_rule(cv, PLUTOVG_FILL_RULE_NON_ZERO);
                addMaskGeometry(cv, t->charId, child, depth + 1);
                plutovg_canvas_clip(cv);
                plutovg_canvas_new_path(cv);
                maskEnds.push_back(t->clipDepth);
                continue;
            }
            const ColorX childCx = key.cx ? cx * fromCx(*key.cx) : cx;
            if (childCx.mult[3] <= 0 && childCx.add[3] <= 0) {
                continue;
            }
            drawCharacter(cv, t->charId, 0, child, childCx, depth + 1);
        }
        while (!maskEnds.empty()) {
            plutovg_canvas_restore(cv);
            maskEnds.pop_back();
        }
    }

    // Bounds of a character in its own space (shape rects, as Flash's DisplayObject bounds; masks clip).
    Bounds bounds(uint16_t id, int frame, const Xform &m, int depth)
    {
        Bounds out;
        auto kind = index.chars.find(id);
        if (kind == index.chars.end() || depth > 32) {
            return out;
        }
        if (kind->second == KindShape) {
            const Rect &r = shape(id)->bounds;
            const float xs[2] = {static_cast<float>(r.xmin / 20.0), static_cast<float>(r.xmax / 20.0)};
            const float ys[2] = {static_cast<float>(r.ymin / 20.0), static_cast<float>(r.ymax / 20.0)};
            for (float x : xs) {
                for (float y : ys) {
                    float px = 0, py = 0;
                    m.apply(x, y, px, py);
                    out.add(px, py);
                }
            }
            return out;
        }
        if (kind->second != KindSprite) {
            return out;
        }
        const TimelineData *tl = timeline(id);
        if (!tl) {
            return out;
        }
        for (const auto &t : tl->tracks) {
            if (frame < t->first || frame > t->last || t->clipDepth > 0) {
                continue;
            }
            const Bounds b = bounds(t->charId, 0, m * fromMatrix(keyAt(*t, frame).matrix), depth + 1);
            if (!b.empty()) {
                out.add(b.x0, b.y0);
                out.add(b.x1, b.y1);
            }
        }
        return out;
    }
};

Rasterizer::Rasterizer(const Swf &swf, const SwfIndex &index) : impl_(std::make_unique<Impl>(swf, index))
{
}

Rasterizer::~Rasterizer() = default;

namespace {

bool savePng(plutovg_surface_t *s, const std::string &path)
{
    return plutovg_surface_write_to_png(s, path.c_str());
}

} // namespace

bool Rasterizer::exportShapes(const std::string &dir, int scale, const std::function<bool(size_t, size_t)> &tick)
{
    const std::string out = dir + "/shapes/" + std::to_string(scale) + "x/";
    SDL_CreateDirectory(out.c_str());
    std::vector<uint16_t> ids;
    for (const auto &[id, kind] : impl_->index.chars) {
        if (kind == KindShape) {
            ids.push_back(id);
        }
    }
    const float s = static_cast<float>(scale);
    std::atomic<size_t> next{0}, done{0};
    std::atomic<bool> stop{false}, failed{false};
    std::mutex tickMutex;
    // Each worker parses its own shapes (no shared cache), so only the bitmap cache needs a lock.
    auto work = [&]() {
        while (!stop) {
            const size_t i = next++;
            if (i >= ids.size()) {
                return;
            }
            Shape shape;
            auto def = impl_->index.defs.find(ids[i]);
            if (def == impl_->index.defs.end() || !ShapeParser(def->second).parse(shape)) {
                SDL_Log("Import: shape %u could not be read", ids[i]);
            }
            const Rect &r = shape.bounds;
            const int w = std::max(1, static_cast<int>(std::ceil((r.xmax - r.xmin) / 20.0 * scale)));
            const int h = std::max(1, static_cast<int>(std::ceil((r.ymax - r.ymin) / 20.0 * scale)));
            if (w <= 8192 && h <= 8192) {
                plutovg_surface_t *surface = plutovg_surface_create(w, h);
                plutovg_canvas_t *cv = plutovg_canvas_create(surface);
                const Xform m{s, 0, 0, s, static_cast<float>(-r.xmin / 20.0) * s, static_cast<float>(-r.ymin / 20.0) * s};
                impl_->drawShape(cv, shape, m, ColorX{});
                plutovg_canvas_destroy(cv);
                const bool ok = savePng(surface, out + std::to_string(ids[i]) + ".png");
                plutovg_surface_destroy(surface);
                if (!ok) {
                    SDL_Log("Import: cannot write shape %u", ids[i]);
                    failed = true;
                    stop = true;
                }
            } else {
                SDL_Log("Import: shape %u too large (%dx%d), skipped", ids[i], w, h);
            }
            const size_t count = ++done;
            if (tick) {
                std::lock_guard<std::mutex> lock(tickMutex);
                if (!tick(count, ids.size())) {
                    stop = true;
                }
            }
        }
    };
    const unsigned threads = std::clamp(std::thread::hardware_concurrency(), 1u, 8u);
    std::vector<std::thread> pool;
    for (unsigned t = 1; t < threads; ++t) {
        pool.emplace_back(work);
    }
    work();
    for (std::thread &t : pool) {
        t.join();
    }
    return !failed && done == ids.size();
}

bool Rasterizer::exportSheets(const std::string &dir, const std::vector<std::string> &names, int scale,
                              const std::function<bool(size_t, size_t)> &tick)
{
    const std::string base = dir + "/sheets/";
    const std::string vecDir = base + std::to_string(scale) + "x/";
    const std::string bmpDir = base + "1x/";
    SDL_CreateDirectory(vecDir.c_str());
    SDL_CreateDirectory(bmpDir.c_str());
    std::string indexText;
    const float s = static_cast<float>(scale);
    for (size_t i = 0; i < names.size(); ++i) {
        const std::string &name = names[i];
        auto cls = impl_->index.classes.find(name);
        if (cls == impl_->index.classes.end()) {
            SDL_Log("Import: sheet class %s not found", name.c_str());
            continue;
        }
        const uint16_t id = cls->second;
        const uint8_t kind = impl_->index.chars.count(id) ? impl_->index.chars.at(id) : 0;
        if (kind == KindBitmap) {
            if (plutovg_surface_t *bmp = impl_->bitmap(id)) {
                savePng(bmp, bmpDir + name + ".png");
                indexText += name + " 0 0 bitmap 0 0\n";
            }
        } else if (kind == KindSprite) {
            const TimelineData *tl = impl_->timeline(id);
            const int last = tl ? std::max(0, static_cast<int>(tl->frameCount) - 1) : 0;
            // loadBitmapFromMovieClip: BitmapData(ceil(width), ceil(height)) measured on the last frame,
            // then frame 1 drawn at the origin.
            const Bounds logical = impl_->bounds(id, last, Xform{}, 0);
            const Bounds content = impl_->bounds(id, 0, Xform{}, 0);
            if (content.empty()) {
                continue;
            }
            const int lw = logical.empty() ? 0 : static_cast<int>(std::ceil(logical.x1 - logical.x0));
            const int lh = logical.empty() ? 0 : static_cast<int>(std::ceil(logical.y1 - logical.y0));
            const int w = std::max(1, static_cast<int>(std::ceil((content.x1 - content.x0) * s)));
            const int h = std::max(1, static_cast<int>(std::ceil((content.y1 - content.y0) * s)));
            if (w > 16384 || h > 16384) {
                SDL_Log("Import: sheet %s too large (%dx%d), skipped", name.c_str(), w, h);
                continue;
            }
            plutovg_surface_t *surface = plutovg_surface_create(w, h);
            plutovg_canvas_t *cv = plutovg_canvas_create(surface);
            const Xform m{s, 0, 0, s, -content.x0 * s, -content.y0 * s};
            impl_->drawCharacter(cv, id, 0, m, ColorX{}, 0);
            plutovg_canvas_destroy(cv);
            savePng(surface, vecDir + name + ".png");
            plutovg_surface_destroy(surface);
            char line[384];
            SDL_snprintf(line, sizeof(line), "%s %g %g vector %d %d\n", name.c_str(), static_cast<double>(content.x0),
                         static_cast<double>(content.y0), lw, lh);
            indexText += line;
        }
        if (tick && !tick(i + 1, names.size())) {
            return false;
        }
    }
    return SDL_SaveFile((base + "index.txt").c_str(), indexText.data(), indexText.size());
}

} // namespace swfimport
