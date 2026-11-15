#pragma once
// Loader for symbols.bin (docs/asset-format.md): characters, shape bounds, class names and timelines.
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "engine/math.h"

namespace engine {

enum class CharKind : uint8_t {
    None = 0, Shape, Sprite, Morph, Text, EditText, Button, Bitmap, Sound, Font
};

struct ShapeBounds {
    float x = 0, y = 0, w = 0, h = 0;
};

struct TimelineKey {
    uint16_t frame = 0;
    bool hasColor = false;
    Affine matrix;
    ColorTransform color;
};

struct Track {
    uint16_t depth = 0;
    uint16_t charId = 0;
    int32_t name = -1;       // index into SymbolLibrary::strings, -1 = unnamed
    uint16_t clipDepth = 0;  // > 0: mask for the depths up to this one
    uint8_t blend = 0;
    uint8_t filters = 0;
    uint16_t first = 0;
    uint16_t last = 0;
    std::vector<TimelineKey> keys;

    bool aliveAt(int frame) const { return frame >= first && frame <= last; }
    const TimelineKey &keyAt(int frame) const;
};

struct Timeline {
    uint16_t id = 0;
    uint16_t frameCount = 1;
    std::vector<std::pair<std::string, uint16_t>> labels;
    std::vector<Track> tracks; // in placement order
};

class SymbolLibrary {
public:
    bool load(const std::string &path);

    CharKind kind(uint16_t id) const;
    const ShapeBounds *shape(uint16_t id) const;
    const Timeline *timeline(uint16_t id) const;
    // Character id linked to an AS3 class name, or -1.
    int findClass(std::string_view name) const;

    const std::vector<std::string> &strings() const { return strings_; }
    const std::vector<std::pair<std::string, uint16_t>> &classes() const { return classes_; }

private:
    std::vector<std::string> strings_;
    std::unordered_map<uint16_t, CharKind> kinds_;
    std::unordered_map<uint16_t, ShapeBounds> shapes_;
    std::unordered_map<uint16_t, Timeline> timelines_;
    std::vector<std::pair<std::string, uint16_t>> classes_;
    std::unordered_map<std::string, uint16_t> classIndex_;
};

} // namespace engine
