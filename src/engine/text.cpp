#include "engine/text.h"

#include <algorithm>
#include <cmath>

#include "engine/json.h"

namespace engine {

namespace {

SDL_FColor jsonColor(const JsonValue *v)
{
    if (!v || v->items.size() < 4) {
        return {0, 0, 0, 1};
    }
    return {static_cast<float>(v->items[0].number), static_cast<float>(v->items[1].number),
            static_cast<float>(v->items[2].number), static_cast<float>(v->items[3].number)};
}

float number(const JsonValue &obj, const char *key, float fallback = 0)
{
    const JsonValue *v = obj.get(key);
    return v ? static_cast<float>(v->number) : fallback;
}

SDL_FColor tint(SDL_FColor c, const ColorTransform &k)
{
    auto ch = [](float v, float mult, float add) { return std::clamp(v * mult + add, 0.0f, 1.0f); };
    return {ch(c.r, k.mr, k.ar), ch(c.g, k.mg, k.ag), ch(c.b, k.mb, k.ab), ch(c.a, k.ma, k.aa)};
}

// Decodes UTF-8 into code points (invalid bytes pass through as themselves).
std::vector<uint32_t> codePoints(std::string_view s)
{
    std::vector<uint32_t> out;
    for (size_t i = 0; i < s.size();) {
        const auto c = static_cast<unsigned char>(s[i]);
        int extra = c >= 0xF0 ? 3 : c >= 0xE0 ? 2 : c >= 0xC0 ? 1 : 0;
        uint32_t cp = extra == 0 ? c : (c & (0x3F >> extra));
        if (i + extra >= s.size()) {
            extra = 0;
            cp = c;
        }
        for (int k = 1; k <= extra; ++k) {
            cp = (cp << 6) | (static_cast<unsigned char>(s[i + k]) & 0x3F);
        }
        out.push_back(cp);
        i += 1 + extra;
    }
    return out;
}

std::string attribute(std::string_view tag, std::string_view name)
{
    const size_t p = tag.find(std::string(name) + "=");
    if (p == std::string_view::npos) {
        return {};
    }
    size_t q = p + name.size() + 1;
    const char quote = q < tag.size() ? tag[q] : 0;
    if (quote != '"' && quote != '\'') {
        return {};
    }
    const size_t end = tag.find(quote, q + 1);
    return std::string(tag.substr(q + 1, end == std::string_view::npos ? std::string_view::npos : end - q - 1));
}

} // namespace

bool TextLibrary::init(SDL_Renderer *renderer, const std::string &gameDir)
{
    clear();
    renderer_ = renderer;
    dir_ = gameDir;
    size_t size = 0;
    char *raw = static_cast<char *>(SDL_LoadFile((gameDir + "/text.json").c_str(), &size));
    if (!raw) {
        SDL_Log("Text: %s/text.json missing (import the game again)", gameDir.c_str());
        return false;
    }
    JsonValue json;
    const bool ok = parseJson(std::string_view(raw, size), json);
    SDL_free(raw);
    if (!ok) {
        return false;
    }
    em_ = number(json, "em", 64);
    atlasScale_ = number(json, "scale", 1);
    if (const JsonValue *fonts = json.get("fonts")) {
        for (const auto &[key, v] : fonts->fields) {
            Font f;
            if (const JsonValue *n = v.get("name")) {
                f.name = n->string;
            }
            f.bold = v.get("bold") && v.get("bold")->boolean;
            f.italic = v.get("italic") && v.get("italic")->boolean;
            f.ascent = number(v, "ascent");
            f.descent = number(v, "descent");
            f.leading = number(v, "leading");
            if (const JsonValue *glyphs = v.get("glyphs")) {
                for (const JsonValue &g : glyphs->items) {
                    if (g.items.size() < 8) {
                        continue;
                    }
                    Glyph glyph;
                    glyph.code = static_cast<uint32_t>(g.items[0].number);
                    glyph.advance = static_cast<float>(g.items[1].number);
                    glyph.x = static_cast<float>(g.items[2].number);
                    glyph.y = static_cast<float>(g.items[3].number);
                    glyph.w = static_cast<float>(g.items[4].number);
                    glyph.h = static_cast<float>(g.items[5].number);
                    glyph.ox = static_cast<float>(g.items[6].number);
                    glyph.oy = static_cast<float>(g.items[7].number);
                    f.byCode[glyph.code] = static_cast<int>(f.glyphs.size());
                    f.glyphs.push_back(glyph);
                }
            }
            fonts_[static_cast<uint16_t>(std::stoi(key))] = std::move(f);
        }
    }
    if (const JsonValue *texts = json.get("texts")) {
        for (const auto &[key, v] : texts->fields) {
            Static s;
            if (const JsonValue *m = v.get("matrix"); m && m->items.size() == 6) {
                s.matrix = {static_cast<float>(m->items[0].number), static_cast<float>(m->items[1].number),
                            static_cast<float>(m->items[2].number), static_cast<float>(m->items[3].number),
                            static_cast<float>(m->items[4].number), static_cast<float>(m->items[5].number)};
            }
            if (const JsonValue *records = v.get("records")) {
                for (const JsonValue &r : records->items) {
                    Run run;
                    run.font = static_cast<uint16_t>(number(r, "font"));
                    run.height = number(r, "height");
                    run.color = jsonColor(r.get("color"));
                    run.x = number(r, "x");
                    run.y = number(r, "y");
                    if (const JsonValue *glyphs = r.get("glyphs")) {
                        for (const JsonValue &g : glyphs->items) {
                            if (g.items.size() == 2) {
                                run.glyphs.emplace_back(static_cast<int>(g.items[0].number),
                                                        static_cast<float>(g.items[1].number));
                            }
                        }
                    }
                    s.runs.push_back(std::move(run));
                }
            }
            statics_[static_cast<uint16_t>(std::stoi(key))] = std::move(s);
        }
    }
    if (const JsonValue *edits = json.get("edits")) {
        for (const auto &[key, v] : edits->fields) {
            Edit e;
            if (const JsonValue *b = v.get("bounds"); b && b->items.size() == 4) {
                e.x0 = static_cast<float>(b->items[0].number);
                e.y0 = static_cast<float>(b->items[1].number);
                e.x1 = static_cast<float>(b->items[2].number);
                e.y1 = static_cast<float>(b->items[3].number);
            }
            e.font = static_cast<uint16_t>(number(v, "font"));
            e.height = number(v, "height", 12);
            e.color = jsonColor(v.get("color"));
            e.align = static_cast<int>(number(v, "align"));
            e.left = number(v, "left");
            e.right = number(v, "right");
            e.indent = number(v, "indent");
            e.leading = number(v, "leading");
            e.wordWrap = v.get("wordWrap") && v.get("wordWrap")->boolean;
            e.multiline = v.get("multiline") && v.get("multiline")->boolean;
            e.html = v.get("html") && v.get("html")->boolean;
            if (const JsonValue *t = v.get("text")) {
                e.text = t->string;
            }
            edits_[static_cast<uint16_t>(std::stoi(key))] = std::move(e);
        }
    }
    SDL_Log("Text: %zu fonts, %zu static texts, %zu text fields", fonts_.size(), statics_.size(), edits_.size());
    return true;
}

void TextLibrary::clear()
{
    for (auto &[id, f] : fonts_) {
        if (f.texture) {
            SDL_DestroyTexture(f.texture);
        }
    }
    fonts_.clear();
    statics_.clear();
    edits_.clear();
}

TextLibrary::Font *TextLibrary::font(uint16_t id)
{
    auto it = fonts_.find(id);
    if (it == fonts_.end()) {
        return nullptr;
    }
    Font &f = it->second;
    if (!f.loaded) {
        f.loaded = true;
        const std::string path = dir_ + "/fonts/" + std::to_string(static_cast<int>(atlasScale_)) + "x/" +
                                 std::to_string(id) + ".png";
        if (SDL_Surface *s = SDL_LoadPNG(path.c_str())) {
            f.texture = SDL_CreateTextureFromSurface(renderer_, s);
            SDL_DestroySurface(s);
        } else {
            SDL_Log("Text: cannot load %s", path.c_str());
        }
    }
    return &f;
}

// The same font family in another face or weight, when the SWF embeds it (HTML <b> / <font face>).
TextLibrary::Font *TextLibrary::variant(uint16_t id, const std::string &face, bool bold)
{
    Font *base = font(id);
    const std::string &name = face.empty() || !base ? (base ? base->name : face) : face;
    if (base && base->name == name && base->bold == bold) {
        return base;
    }
    for (auto &[otherID, f] : fonts_) {
        if (f.name == name && f.bold == bold && !f.glyphs.empty()) {
            return font(otherID);
        }
    }
    for (auto &[otherID, f] : fonts_) {
        if (f.name == name && !f.glyphs.empty()) {
            return font(otherID);
        }
    }
    return base;
}

void TextLibrary::drawGlyph(const Font &f, const Glyph &g, float penX, float baseline, float size, const Affine &m,
                            SDL_FColor color)
{
    if (!f.texture || g.w <= 0) {
        return;
    }
    float tw = 0, th = 0;
    SDL_GetTextureSize(f.texture, &tw, &th);
    const float k = size / em_ / atlasScale_; // local pixels per atlas pixel
    const float x0 = penX - g.ox * k, y0 = baseline - g.oy * k;
    const float x1 = x0 + g.w * k, y1 = y0 + g.h * k;
    const float corners[4][4] = {{x0, y0, g.x / tw, g.y / th},
                                 {x1, y0, (g.x + g.w) / tw, g.y / th},
                                 {x1, y1, (g.x + g.w) / tw, (g.y + g.h) / th},
                                 {x0, y1, g.x / tw, (g.y + g.h) / th}};
    SDL_Vertex v[4];
    for (int i = 0; i < 4; ++i) {
        m.apply(corners[i][0], corners[i][1], v[i].position.x, v[i].position.y);
        v[i].color = color;
        v[i].tex_coord = {corners[i][2], corners[i][3]};
    }
    static const int indices[6] = {0, 1, 2, 0, 2, 3};
    SDL_RenderGeometry(renderer_, f.texture, v, 4, indices, 6);
}

void TextLibrary::drawStatic(uint16_t id, const Affine &m, const ColorTransform &color)
{
    auto it = statics_.find(id);
    if (it == statics_.end()) {
        return;
    }
    const Affine local = m * it->second.matrix;
    for (const Run &run : it->second.runs) {
        Font *f = font(run.font);
        if (!f) {
            continue;
        }
        float pen = run.x;
        for (const auto &[index, advance] : run.glyphs) {
            if (index >= 0 && index < static_cast<int>(f->glyphs.size())) {
                drawGlyph(*f, f->glyphs[static_cast<size_t>(index)], pen, run.y, run.height, local,
                          tint(run.color, color));
            }
            pen += advance;
        }
    }
}

// Plain text of an HTML text field, and the style of its first tags (enough for the game's one-style fields).
std::string TextLibrary::parseHtml(std::string_view html, Style &style)
{
    std::string out;
    for (size_t i = 0; i < html.size();) {
        if (html[i] == '<') {
            const size_t end = html.find('>', i);
            if (end == std::string_view::npos) {
                break;
            }
            const std::string_view tag = html.substr(i + 1, end - i - 1);
            const std::string lower = [&] {
                std::string s(tag);
                std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
                return s;
            }();
            if (lower == "b") {
                style.bold = true;
            } else if (lower.starts_with("p ") && style.align < 0) {
                const std::string a = attribute(lower, "align");
                style.align = a == "right" ? 1 : a == "center" ? 2 : a == "justify" ? 3 : 0;
            } else if (lower.starts_with("font ")) {
                if (const std::string s = attribute(lower, "size"); !s.empty() && style.size < 0) {
                    style.size = std::strtof(s.c_str(), nullptr);
                }
                if (const std::string c = attribute(lower, "color"); c.size() == 7 && !style.hasColor) {
                    const unsigned long rgb = std::strtoul(c.c_str() + 1, nullptr, 16);
                    style.color = {((rgb >> 16) & 255) / 255.0f, ((rgb >> 8) & 255) / 255.0f, (rgb & 255) / 255.0f, 1};
                    style.hasColor = true;
                }
                if (const std::string f = attribute(std::string(tag), "face"); !f.empty() && style.face.empty()) {
                    style.face = f;
                }
            } else if ((lower == "br" || lower == "/p") && !out.empty()) {
                out += '\n';
            }
            i = end + 1;
        } else if (html[i] == '&') {
            static const std::pair<const char *, const char *> entities[] = {
                {"&amp;", "&"}, {"&lt;", "<"}, {"&gt;", ">"}, {"&quot;", "\""}, {"&apos;", "'"}, {"&nbsp;", " "}};
            bool matched = false;
            for (const auto &[name, value] : entities) {
                if (html.substr(i).starts_with(name)) {
                    out += value;
                    i += std::string_view(name).size();
                    matched = true;
                    break;
                }
            }
            if (!matched) {
                out += html[i++];
            }
        } else {
            out += html[i++];
        }
    }
    while (!out.empty() && out.back() == '\n') {
        out.pop_back();
    }
    return out;
}

void TextLibrary::drawEdit(uint16_t id, const std::string *text, const Affine &m, const ColorTransform &color)
{
    auto it = edits_.find(id);
    if (it == edits_.end()) {
        return;
    }
    const Edit &e = it->second;
    const std::string &source = text ? *text : e.text;
    Style style;
    const bool html = e.html || (!source.empty() && source[0] == '<');
    const std::string plain = html ? parseHtml(source, style) : source;
    Font *f = variant(e.font, style.face, style.bold || (font(e.font) && font(e.font)->bold));
    if (!f || plain.empty()) {
        return;
    }
    const float size = style.size > 0 ? style.size : e.height;
    const int align = style.align >= 0 ? style.align : e.align;
    const SDL_FColor c = tint(style.hasColor ? style.color : e.color, color);
    const float k = size / em_;
    // Lines: explicit breaks, then word wrap when the field wraps.
    std::vector<std::vector<uint32_t>> lines(1);
    for (uint32_t cp : codePoints(plain)) {
        if (cp == '\n' || cp == '\r') {
            lines.emplace_back();
        } else {
            lines.back().push_back(cp);
        }
    }
    auto advanceOf = [&](uint32_t cp) {
        auto g = f->byCode.find(cp);
        return g == f->byCode.end() ? 0.0f : f->glyphs[static_cast<size_t>(g->second)].advance * k;
    };
    const float avail = (e.x1 - e.x0) - 4 - e.left - e.right;
    if (e.wordWrap) {
        std::vector<std::vector<uint32_t>> wrapped;
        for (const auto &line : lines) {
            std::vector<uint32_t> cur;
            float width = 0;
            size_t lastSpace = SIZE_MAX;
            for (uint32_t cp : line) {
                cur.push_back(cp);
                width += advanceOf(cp);
                if (cp == ' ') {
                    lastSpace = cur.size() - 1;
                }
                if (width > avail && lastSpace != SIZE_MAX) {
                    wrapped.emplace_back(cur.begin(), cur.begin() + static_cast<std::ptrdiff_t>(lastSpace));
                    cur.erase(cur.begin(), cur.begin() + static_cast<std::ptrdiff_t>(lastSpace) + 1);
                    width = 0;
                    for (uint32_t c2 : cur) {
                        width += advanceOf(c2);
                    }
                    lastSpace = SIZE_MAX;
                }
            }
            wrapped.push_back(std::move(cur));
        }
        lines = std::move(wrapped);
    }
    float baseline = e.y0 + 2 + f->ascent * k;
    for (size_t li = 0; li < lines.size(); ++li) {
        const auto &line = lines[li];
        float width = 0;
        for (uint32_t cp : line) {
            width += advanceOf(cp);
        }
        float pen = e.x0 + 2 + e.left + (li == 0 ? e.indent : 0);
        if (align == 1) {
            pen += avail - width;
        } else if (align == 2) {
            pen += (avail - width) / 2;
        }
        for (uint32_t cp : line) {
            auto g = f->byCode.find(cp);
            if (g != f->byCode.end()) {
                const Glyph &glyph = f->glyphs[static_cast<size_t>(g->second)];
                drawGlyph(*f, glyph, pen, baseline, size, m, c);
                pen += glyph.advance * k;
            }
        }
        baseline += (f->ascent + f->descent + f->leading) * k + e.leading;
    }
}

// The font with this name that embeds the most glyphs.
TextLibrary::Font *TextLibrary::fontByName(std::string_view name)
{
    uint16_t best = 0;
    size_t most = 0;
    for (auto &[id, f] : fonts_) {
        if (f.name == name && f.glyphs.size() > most) {
            best = id;
            most = f.glyphs.size();
        }
    }
    return most ? font(best) : nullptr;
}

float TextLibrary::drawString(std::string_view fontName, float size, std::string_view utf8, float x, float baseline,
                              SDL_FColor color, int align, const Affine &m, bool draw)
{
    Font *f = fontByName(fontName);
    if (!f) {
        return 0;
    }
    // Letters missing from the game's fonts: base letter + mark (acute ', tilde ˜, diaeresis ..), or a
    // letter turned upside down (¿ ¡).
    struct Compose {
        uint32_t code, base;
        char mark; // '\'' acute, '~' tilde, ':' diaeresis, 'R' rotate the base 180 degrees
    };
    static const Compose kCompose[] = {
        {0xE1, 'a', '\''}, {0xE9, 'e', '\''}, {0xED, 'i', '\''}, {0xF3, 'o', '\''}, {0xFA, 'u', '\''},
        {0xC1, 'A', '\''}, {0xC9, 'E', '\''}, {0xCD, 'I', '\''}, {0xD3, 'O', '\''}, {0xDA, 'U', '\''},
        {0xF1, 'n', '~'},  {0xD1, 'N', '~'},  {0xFC, 'u', ':'},  {0xDC, 'U', ':'},
        {0xBF, '?', 'R'},  {0xA1, '!', 'R'},
    };
    auto glyphOf = [&](uint32_t code) -> const Glyph * {
        auto it = f->byCode.find(code);
        return it == f->byCode.end() ? nullptr : &f->glyphs[static_cast<size_t>(it->second)];
    };
    const float k = size / em_;
    // Width first (for the alignment), then draw.
    float width = 0;
    const std::vector<uint32_t> codes = codePoints(utf8);
    for (uint32_t c : codes) {
        const Glyph *g = glyphOf(c);
        if (!g) {
            for (const Compose &cp : kCompose) {
                if (cp.code == c) {
                    g = glyphOf(cp.base);
                }
            }
        }
        if (g) {
            width += g->advance * k;
        }
    }
    if (!draw) {
        return width;
    }
    float pen = align == 1 ? x - width : (align == 2 ? x - width / 2 : x);
    const float xHeight = f->ascent * k * 0.52f;
    for (uint32_t c : codes) {
        if (const Glyph *g = glyphOf(c)) {
            drawGlyph(*f, *g, pen, baseline, size, m, color);
            pen += g->advance * k;
            continue;
        }
        const Compose *cp = nullptr;
        for (const Compose &item : kCompose) {
            if (item.code == c) {
                cp = &item;
            }
        }
        const Glyph *base = cp ? glyphOf(cp->base) : nullptr;
        if (!base) {
            continue;
        }
        const float advance = base->advance * k;
        if (cp->mark == 'R') {
            // Turned 180 degrees about the middle of the letter's x-height band.
            const float cx = pen + advance / 2, cy = baseline - xHeight * 0.75f;
            const Affine turn = m * Affine{-1, 0, 0, -1, 2 * cx, 2 * cy};
            drawGlyph(*f, *base, pen, baseline + xHeight * 0.5f, size, turn, color);
        } else {
            drawGlyph(*f, *base, pen, baseline, size, m, color);
            // The apostrophe and the tilde already sit at cap height: over a lowercase letter they need no
            // move, over a capital they go up by about the cap height minus the x-height.
            const bool capital = cp->base >= 'A' && cp->base <= 'Z';
            const float lift = capital ? size * 0.3f : 0.0f;
            if (cp->mark == '\'') {
                if (const Glyph *mark = glyphOf('\'')) {
                    // Slanted like an acute accent (top to the right).
                    const float mw = mark->advance * k;
                    const float markBase = baseline - lift;
                    const Affine shear = m * Affine{1, 0, -0.5f, 1, 0.5f * (markBase - size * 0.5f), 0};
                    drawGlyph(*f, *mark, pen + (advance - mw) / 2, markBase, size, shear, color);
                }
            } else if (cp->mark == '~') {
                if (const Glyph *mark = glyphOf(0x2DC)) {
                    const float mw = mark->advance * k;
                    drawGlyph(*f, *mark, pen + (advance - mw) / 2, baseline - lift, size, m, color);
                }
            } else if (cp->mark == ':') {
                if (const Glyph *dot = glyphOf('.')) {
                    const float dw = dot->advance * k;
                    const float dotBase = baseline - (capital ? size * 0.82f : size * 0.6f);
                    drawGlyph(*f, *dot, pen + advance / 2 - dw * 1.05f, dotBase, size, m, color);
                    drawGlyph(*f, *dot, pen + advance / 2 + dw * 0.05f, dotBase, size, m, color);
                }
            }
        }
        pen += advance;
    }
    return width;
}

} // namespace engine
