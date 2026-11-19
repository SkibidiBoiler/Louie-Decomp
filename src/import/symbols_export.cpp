#include "import/symbols_export.h"

#include <algorithm>
#include <cstring>
#include <memory>
#include <optional>
#include <unordered_map>

namespace swfimport {

namespace {

uint8_t kindForTag(uint16_t code)
{
    switch (code) {
    case 2: case 22: case 32: case 83: return KindShape;
    case 39: return KindSprite;
    case 46: case 84: return KindMorph;
    case 11: case 33: return KindText;
    case 37: return KindEditText;
    case 7: case 34: return KindButton;
    case 6: case 20: case 21: case 35: case 36: case 90: return KindBitmap;
    case 14: return KindSound;
    case 10: case 48: case 75: case 91: return KindFont;
    default: return 0;
    }
}

size_t skipFilters(const uint8_t *d, size_t size, size_t pos, uint8_t &count)
{
    if (pos >= size) {
        count = 0;
        return pos;
    }
    count = d[pos++];
    for (int i = 0; i < count && pos < size; ++i) {
        const uint8_t id = d[pos++];
        switch (id) {
        case 0: pos += 23; break;           // drop shadow
        case 1: pos += 9; break;            // blur
        case 2: pos += 15; break;           // glow
        case 3: pos += 27; break;           // bevel
        case 4: case 7: {                   // gradient glow / gradient bevel
            const uint8_t n = pos < size ? d[pos] : 0;
            pos += 1 + 5u * n + 19;
            break;
        }
        case 5: {                           // convolution
            const uint8_t mx = d[pos], my = d[pos + 1];
            pos += 2 + 8 + 4u * mx * my + 4 + 1;
            break;
        }
        case 6: pos += 80; break;           // colour matrix
        default: return size;               // unknown: give up on the rest of the record
        }
    }
    return pos;
}

} // namespace

TimelineData parseTimeline(const uint8_t *data, size_t pos, size_t end, uint16_t frameCount)
{
    TimelineData tl;
    tl.frameCount = frameCount;
    std::unordered_map<uint16_t, TrackData *> live;
    int frame = 0;
    forEachTag(data, pos, end, [&](const Tag &tag) {
        const uint8_t *b = tag.body;
        const size_t n = tag.size;
        if (tag.code == 1) { // ShowFrame
            ++frame;
        } else if (tag.code == 43) { // FrameLabel
            size_t p = 0;
            std::string label = cstring(b, n, p);
            auto it = std::find_if(tl.labels.begin(), tl.labels.end(), [&](auto &l) { return l.first == label; });
            if (it != tl.labels.end()) {
                it->second = static_cast<uint16_t>(frame);
            } else {
                tl.labels.emplace_back(std::move(label), static_cast<uint16_t>(frame));
            }
        } else if (tag.code == 5 || tag.code == 28) { // RemoveObject(2)
            const size_t off = tag.code == 5 ? 2 : 0;
            if (n < off + 2) {
                return;
            }
            auto it = live.find(u16(b + off));
            if (it != live.end()) {
                it->second->last = frame - 1;
                live.erase(it);
            }
        } else if (tag.code == 26 || tag.code == 70) { // PlaceObject2 / PlaceObject3
            const bool po3 = tag.code == 70;
            const uint8_t f1 = b[0];
            const uint8_t f2 = po3 ? b[1] : 0;
            size_t p = po3 ? 2 : 1;
            const uint16_t depth = u16(b + p);
            p += 2;
            if (po3 && ((f2 & 0x08) || ((f2 & 0x10) && (f1 & 0x02)))) {
                cstring(b, n, p); // class name
            }
            std::optional<uint16_t> charId;
            if (f1 & 0x02) {
                charId = u16(b + p);
                p += 2;
            }
            std::optional<Matrix> matrix;
            std::optional<CxForm> cx;
            std::optional<uint16_t> ratio;
            std::optional<std::string> name;
            uint16_t clip = 0;
            if (f1 & 0x04) {
                matrix = readMatrix(b, n, p);
            }
            if (f1 & 0x08) {
                cx = readCxForm(b, n, p, true);
            }
            if (f1 & 0x10) {
                ratio = u16(b + p);
                p += 2;
            }
            if (f1 & 0x20) {
                name = cstring(b, n, p);
            }
            if (f1 & 0x40) {
                clip = u16(b + p);
                p += 2;
            }
            uint8_t blend = 0, nfilters = 0;
            if (po3) {
                if (f2 & 0x01) {
                    p = skipFilters(b, n, p, nfilters);
                }
                if ((f2 & 0x02) && p < n) {
                    blend = b[p++];
                }
            }
            auto liveIt = live.find(depth);
            TrackData *t = liveIt != live.end() ? liveIt->second : nullptr;
            if (charId && (t == nullptr || !(f1 & 0x01) || t->charId != *charId)) {
                TrackData *old = t;
                if (old) {
                    old->last = frame - 1;
                }
                auto fresh = std::make_unique<TrackData>();
                fresh->depth = depth;
                fresh->charId = *charId;
                fresh->first = frame;
                fresh->name = name.value_or("");
                fresh->clipDepth = clip;
                fresh->blend = blend;
                fresh->filters = nfilters;
                if (old && (f1 & 0x01)) { // character swap keeps the old transform
                    fresh->matrix = old->matrix;
                    fresh->cx = old->cx;
                    fresh->ratio = old->ratio;
                    if (fresh->name.empty()) {
                        fresh->name = old->name;
                    }
                }
                t = fresh.get();
                live[depth] = t;
                tl.tracks.push_back(std::move(fresh));
            } else if (t == nullptr) {
                return;
            }
            if (matrix) {
                t->matrix = *matrix;
            }
            if (cx) {
                t->cx = cx;
            }
            if (ratio) {
                t->ratio = ratio;
            }
            if (name) {
                t->name = *name;
            }
            if (blend) {
                t->blend = blend;
            }
            if (!t->keys.empty() && t->keys.back().frame == frame) {
                t->keys.pop_back();
            }
            t->keys.push_back(Key{static_cast<uint16_t>(frame), t->matrix, t->cx, t->ratio});
        }
    });
    for (auto &t : tl.tracks) {
        if (t->last < 0 && live.count(t->depth) && live[t->depth] == t.get()) {
            t->last = std::max(static_cast<int>(frameCount) - 1, t->first);
        }
        t->last = std::max(t->last, t->first);
    }
    return tl;
}

namespace {

// DefineButton (7) / DefineButton2 (34) as a four-frame timeline: 0 up, 1 over, 2 down, 3 hit area. A record
// shown in states that are not adjacent becomes one track per run of states.
TimelineData parseButton(const Tag &tag)
{
    TimelineData tl;
    tl.frameCount = 4;
    const uint8_t *b = tag.body;
    const size_t n = tag.size;
    const bool v2 = tag.code == 34;
    size_t p = v2 ? 5 : 2; // id (+ flags, action offset)
    while (p < n && b[p] != 0) {
        const uint8_t flags = b[p++];
        if (p + 4 > n) {
            break;
        }
        const uint16_t charId = u16(b + p);
        const uint16_t depth = u16(b + p + 2);
        p += 4;
        const Matrix matrix = readMatrix(b, n, p);
        std::optional<CxForm> cx;
        if (v2) {
            cx = readCxForm(b, n, p, true);
        }
        uint8_t nfilters = 0, blend = 0;
        if (v2 && (flags & 0x10)) {
            p = skipFilters(b, n, p, nfilters);
        }
        if (v2 && (flags & 0x20) && p < n) {
            blend = b[p++];
        }
        const bool states[4] = {(flags & 1) != 0, (flags & 2) != 0, (flags & 4) != 0, (flags & 8) != 0};
        for (int s = 0; s < 4; ++s) {
            if (!states[s] || (s > 0 && states[s - 1])) {
                continue;
            }
            int e = s;
            while (e + 1 < 4 && states[e + 1]) {
                ++e;
            }
            auto t = std::make_unique<TrackData>();
            t->depth = depth;
            t->charId = charId;
            t->first = s;
            t->last = e;
            t->matrix = matrix;
            t->cx = cx;
            t->blend = blend;
            t->filters = nfilters;
            t->keys.push_back(Key{static_cast<uint16_t>(s), matrix, cx, std::nullopt});
            tl.tracks.push_back(std::move(t));
        }
    }
    return tl;
}

class Writer {
public:
    std::vector<uint8_t> bytes;
    void u8(uint8_t v) { bytes.push_back(v); }
    void u16(uint16_t v) { u8(static_cast<uint8_t>(v)); u8(static_cast<uint8_t>(v >> 8)); }
    void u32(uint32_t v) { u16(static_cast<uint16_t>(v)); u16(static_cast<uint16_t>(v >> 16)); }
    void f32(float v) { uint32_t u; std::memcpy(&u, &v, 4); u32(u); }
    void raw(const void *p, size_t n) { auto *c = static_cast<const uint8_t *>(p); bytes.insert(bytes.end(), c, c + n); }
};

class Strings {
public:
    uint32_t add(const std::string &s)
    {
        auto it = index_.find(s);
        if (it != index_.end()) {
            return it->second;
        }
        const auto id = static_cast<uint32_t>(items.size());
        index_[s] = id;
        items.push_back(s);
        return id;
    }
    std::vector<std::string> items;

private:
    std::unordered_map<std::string, uint32_t> index_;
};

} // namespace

SwfIndex indexSwf(const Swf &swf)
{
    SwfIndex index;
    const auto &d = swf.data();
    forEachTag(d.data(), swf.firstTag(), d.size(), [&](const Tag &tag) {
        if (const uint8_t kind = kindForTag(tag.code); kind && tag.size >= 2) {
            const uint16_t id = u16(tag.body);
            index.chars[id] = kind;
            index.defs[id] = tag;
        } else if (tag.code == 76 && tag.size >= 2) { // SymbolClass
            const uint16_t n = u16(tag.body);
            size_t p = 2;
            for (uint16_t i = 0; i < n && p + 2 <= tag.size; ++i) {
                const uint16_t id = u16(tag.body + p);
                p += 2;
                index.classes[cstring(tag.body, tag.size, p)] = id;
            }
        } else if (tag.code == 82 || tag.code == 72) { // DoABC2 / DoABC
            index.abc.push_back(tag);
        }
    });
    return index;
}

std::vector<uint8_t> buildSymbols(const Swf &swf, const SwfIndex &index)
{
    const auto &d = swf.data();
    std::map<uint16_t, TimelineData> sprites;
    for (const auto &[id, tag] : index.defs) {
        if (index.chars.at(id) == KindSprite && tag.size >= 4) {
            const uint16_t fc = u16(tag.body + 2);
            sprites[id] = parseTimeline(tag.body, 4, tag.size, fc);
        } else if (index.chars.at(id) == KindButton) {
            sprites[id] = parseButton(tag);
        }
    }
    sprites[0] = parseTimeline(d.data(), swf.firstTag(), d.size(), swf.frameCount());

    Strings strings;
    Writer chars;
    chars.u32(static_cast<uint32_t>(index.chars.size()));
    for (const auto &[id, kind] : index.chars) {
        chars.u16(id);
        chars.u8(kind);
        if (kind == KindShape) {
            const Tag &tag = index.defs.at(id);
            size_t p = 2;
            const Rect r = readRect(tag.body, tag.size, p);
            chars.f32(static_cast<float>(r.xmin / 20.0));
            chars.f32(static_cast<float>(r.ymin / 20.0));
            chars.f32(static_cast<float>((r.xmax - r.xmin) / 20.0));
            chars.f32(static_cast<float>((r.ymax - r.ymin) / 20.0));
        }
    }
    Writer classes;
    classes.u32(static_cast<uint32_t>(index.classes.size()));
    for (const auto &[name, id] : index.classes) {
        classes.u32(strings.add(name));
        classes.u16(id);
    }
    Writer sp;
    sp.u32(static_cast<uint32_t>(sprites.size()));
    for (auto &[id, tl] : sprites) {
        sp.u16(id);
        sp.u16(tl.frameCount);
        auto labels = tl.labels;
        std::stable_sort(labels.begin(), labels.end(), [](auto &a, auto &b) { return a.second < b.second; });
        sp.u16(static_cast<uint16_t>(labels.size()));
        for (const auto &[label, frame] : labels) {
            sp.u32(strings.add(label));
            sp.u16(frame);
        }
        sp.u32(static_cast<uint32_t>(tl.tracks.size()));
        for (const auto &t : tl.tracks) {
            sp.u16(t->depth);
            sp.u16(t->charId);
            sp.u32(t->name.empty() ? 0xFFFFFFFFu : strings.add(t->name));
            sp.u16(t->clipDepth);
            sp.u8(t->blend);
            sp.u8(t->filters);
            sp.u16(static_cast<uint16_t>(t->first));
            sp.u16(static_cast<uint16_t>(t->last));
            sp.u32(static_cast<uint32_t>(t->keys.size()));
            for (const Key &k : t->keys) {
                sp.u16(k.frame);
                sp.u8(static_cast<uint8_t>((k.cx ? 1 : 0) | (k.ratio ? 2 : 0)));
                const Matrix &m = k.matrix;
                for (float v : {m.a, m.b, m.c, m.d, m.tx, m.ty}) {
                    sp.f32(v);
                }
                if (k.cx) {
                    for (float v : k.cx->mult) {
                        sp.f32(v);
                    }
                    for (float v : k.cx->add) {
                        sp.f32(v);
                    }
                }
                if (k.ratio) {
                    sp.u16(*k.ratio);
                }
            }
        }
    }
    Writer out;
    out.raw("PLSY", 4);
    out.u32(1);
    out.u32(static_cast<uint32_t>(strings.items.size()));
    for (const std::string &s : strings.items) {
        out.u16(static_cast<uint16_t>(s.size()));
        out.raw(s.data(), s.size());
    }
    out.raw(chars.bytes.data(), chars.bytes.size());
    out.raw(classes.bytes.data(), classes.bytes.size());
    out.raw(sp.bytes.data(), sp.bytes.size());
    return std::move(out.bytes);
}

} // namespace swfimport
