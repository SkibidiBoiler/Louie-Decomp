#include "engine/symbols.h"

#include <SDL3/SDL.h>
#include <cstring>

namespace engine {

namespace {

class Reader {
public:
    Reader(const uint8_t *data, size_t size) : data_(data), size_(size) {}

    bool ok() const { return ok_; }

    template <typename T> T get()
    {
        T value{};
        if (pos_ + sizeof(T) > size_) {
            ok_ = false;
            return value;
        }
        std::memcpy(&value, data_ + pos_, sizeof(T)); // asset files are little-endian, like every target
        pos_ += sizeof(T);
        return value;
    }

    std::string str(size_t len)
    {
        if (pos_ + len > size_) {
            ok_ = false;
            return {};
        }
        std::string s(reinterpret_cast<const char *>(data_ + pos_), len);
        pos_ += len;
        return s;
    }

private:
    const uint8_t *data_;
    size_t size_;
    size_t pos_ = 0;
    bool ok_ = true;
};

constexpr uint32_t kNoName = 0xFFFFFFFFu;

} // namespace

const TimelineKey &Track::keyAt(int frame) const
{
    const TimelineKey *best = &keys.front();
    for (const TimelineKey &k : keys) {
        if (k.frame > frame) {
            break;
        }
        best = &k;
    }
    return *best;
}

bool SymbolLibrary::load(const std::string &path)
{
    size_t size = 0;
    void *raw = SDL_LoadFile(path.c_str(), &size);
    if (!raw) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot read %s: %s", path.c_str(), SDL_GetError());
        return false;
    }
    Reader r(static_cast<const uint8_t *>(raw), size);

    const std::string magic = r.str(4);
    const uint32_t version = r.get<uint32_t>();
    if (magic != "PLSY" || version != 1) {
        SDL_free(raw);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: not a v1 symbols file", path.c_str());
        return false;
    }

    const uint32_t stringCount = r.get<uint32_t>();
    strings_.reserve(stringCount);
    for (uint32_t i = 0; i < stringCount && r.ok(); ++i) {
        strings_.push_back(r.str(r.get<uint16_t>()));
    }

    const uint32_t charCount = r.get<uint32_t>();
    for (uint32_t i = 0; i < charCount && r.ok(); ++i) {
        const uint16_t id = r.get<uint16_t>();
        const auto kind = static_cast<CharKind>(r.get<uint8_t>());
        kinds_[id] = kind;
        if (kind == CharKind::Shape) {
            ShapeBounds b;
            b.x = r.get<float>();
            b.y = r.get<float>();
            b.w = r.get<float>();
            b.h = r.get<float>();
            shapes_[id] = b;
        }
    }

    const uint32_t classCount = r.get<uint32_t>();
    for (uint32_t i = 0; i < classCount && r.ok(); ++i) {
        const uint32_t name = r.get<uint32_t>();
        const uint16_t id = r.get<uint16_t>();
        if (name < strings_.size()) {
            classes_.emplace_back(strings_[name], id);
            classIndex_[strings_[name]] = id;
        }
    }

    const uint32_t timelineCount = r.get<uint32_t>();
    for (uint32_t i = 0; i < timelineCount && r.ok(); ++i) {
        Timeline tl;
        tl.id = r.get<uint16_t>();
        tl.frameCount = r.get<uint16_t>();
        if (tl.frameCount == 0) {
            tl.frameCount = 1;
        }
        const uint16_t labelCount = r.get<uint16_t>();
        for (uint16_t l = 0; l < labelCount && r.ok(); ++l) {
            const uint32_t name = r.get<uint32_t>();
            const uint16_t frame = r.get<uint16_t>();
            tl.labels.emplace_back(name < strings_.size() ? strings_[name] : std::string(), frame);
        }
        const uint32_t trackCount = r.get<uint32_t>();
        tl.tracks.resize(trackCount);
        for (Track &t : tl.tracks) {
            t.depth = r.get<uint16_t>();
            t.charId = r.get<uint16_t>();
            const uint32_t name = r.get<uint32_t>();
            t.name = name == kNoName ? -1 : static_cast<int32_t>(name);
            t.clipDepth = r.get<uint16_t>();
            t.blend = r.get<uint8_t>();
            t.filters = r.get<uint8_t>();
            t.first = r.get<uint16_t>();
            t.last = r.get<uint16_t>();
            const uint32_t keyCount = r.get<uint32_t>();
            t.keys.resize(keyCount);
            for (TimelineKey &k : t.keys) {
                k.frame = r.get<uint16_t>();
                const uint8_t flags = r.get<uint8_t>();
                k.matrix = {r.get<float>(), r.get<float>(), r.get<float>(),
                            r.get<float>(), r.get<float>(), r.get<float>()};
                if (flags & 1) {
                    k.hasColor = true;
                    k.color = {r.get<float>(), r.get<float>(), r.get<float>(), r.get<float>(),
                               r.get<float>(), r.get<float>(), r.get<float>(), r.get<float>()};
                }
                if (flags & 2) {
                    r.get<uint16_t>(); // morph ratio: not used yet
                }
            }
            if (!r.ok()) {
                break;
            }
            if (t.keys.empty()) {
                t.keys.emplace_back();
            }
        }
        if (r.ok()) {
            timelines_[tl.id] = std::move(tl);
        }
    }

    SDL_free(raw);
    if (!r.ok()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: truncated file", path.c_str());
        return false;
    }
    SDL_Log("Symbols: %zu characters, %zu shapes, %zu timelines, %zu classes", kinds_.size(), shapes_.size(),
            timelines_.size(), classes_.size());
    return true;
}

CharKind SymbolLibrary::kind(uint16_t id) const
{
    auto it = kinds_.find(id);
    return it == kinds_.end() ? CharKind::None : it->second;
}

const ShapeBounds *SymbolLibrary::shape(uint16_t id) const
{
    auto it = shapes_.find(id);
    return it == shapes_.end() ? nullptr : &it->second;
}

const Timeline *SymbolLibrary::timeline(uint16_t id) const
{
    auto it = timelines_.find(id);
    return it == timelines_.end() ? nullptr : &it->second;
}

int SymbolLibrary::findClass(std::string_view name) const
{
    auto it = classIndex_.find(std::string(name));
    return it == classIndex_.end() ? -1 : it->second;
}

} // namespace engine
