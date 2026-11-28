#include "engine/symbols.h"

#include <cstdio>

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
                    k.ratio = r.get<uint16_t>();
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

int SymbolLibrary::findShortClass(std::string_view name) const
{
    int found = -1;
    for (const auto &[full, id] : classes_) {
        const size_t dot = full.rfind('.');
        if (dot != std::string::npos && std::string_view(full).substr(dot + 1) == name) {
            if (found >= 0) {
                return -1; // ambiguous
            }
            found = id;
        }
    }
    return found;
}

void SymbolLibrary::loadFrameScripts(const std::string &path)
{
    scripts_.clear();
    size_t size = 0;
    char *raw = static_cast<char *>(SDL_LoadFile(path.c_str(), &size));
    if (!raw) {
        return;
    }
    const std::string text(raw, size);
    SDL_free(raw);
    size_t pos = 0, count = 0;
    while (pos < text.size()) {
        size_t eol = text.find('\n', pos);
        if (eol == std::string::npos) {
            eol = text.size();
        }
        char cls[256] = {}, op[32] = {}, target[256] = {};
        unsigned frame = 0;
        double number = 0;
        const std::string line = text.substr(pos, eol - pos);
        pos = eol + 1;
        if (std::sscanf(line.c_str(), "%255s %u %31s %255s %lf", cls, &frame, op, target, &number) != 5) {
            continue;
        }
        int id = findClass(cls);
        if (id < 0) {
            // Older imports wrote the class name without its package: match it when only one class has it.
            id = findShortClass(cls);
        }
        if (id < 0) {
            continue;
        }
        FrameScriptOp s;
        const std::string o = op;
        if (o == "stop") s.op = FrameScriptOp::Op::Stop;
        else if (o == "play") s.op = FrameScriptOp::Op::Play;
        else if (o == "gotoAndPlay") s.op = FrameScriptOp::Op::GotoAndPlay;
        else if (o == "gotoAndStop") s.op = FrameScriptOp::Op::GotoAndStop;
        else if (o == "child") s.op = FrameScriptOp::Op::Child;
        else continue;
        s.target = std::string(target) == "-" ? std::string() : std::string(target);
        s.number = static_cast<int>(number);
        scripts_[(static_cast<uint32_t>(id) << 16) | (frame & 0xFFFFu)].push_back(std::move(s));
        ++count;
    }
    SDL_Log("Frame scripts: %zu statements", count);
}

const std::vector<FrameScriptOp> *SymbolLibrary::frameScript(uint16_t id, int frame) const
{
    auto it = scripts_.find((static_cast<uint32_t>(id) << 16) | (static_cast<uint32_t>(frame) & 0xFFFFu));
    return it == scripts_.end() ? nullptr : &it->second;
}

int SymbolLibrary::labelFrame(const Timeline &tl, std::string_view label)
{
    for (const auto &l : tl.labels) {
        if (l.first == label) {
            return l.second;
        }
    }
    return -1;
}

} // namespace engine
