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

// One statement of a converted timeline frame script (framescripts.txt, docs/asset-format.md).
struct FrameScriptOp {
    enum class Op : uint8_t { Stop, Play, GotoAndPlay, GotoAndStop, Child } op = Op::Stop;
    std::string target; // label, or the container instance name for Child
    int number = 0;     // 1-based frame for goto/Child
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

    // Reads framescripts.txt (optional). Frame scripts of timeline `id` at 0-based `frame`, or nullptr.
    void loadFrameScripts(const std::string &path);
    const std::vector<FrameScriptOp> *frameScript(uint16_t id, int frame) const;
    // 0-based frame of a label in timeline `tl`, or -1.
    static int labelFrame(const Timeline &tl, std::string_view label);

    const std::vector<std::string> &strings() const { return strings_; }
    const std::vector<std::pair<std::string, uint16_t>> &classes() const { return classes_; }

private:
    std::vector<std::string> strings_;
    std::unordered_map<uint16_t, CharKind> kinds_;
    std::unordered_map<uint16_t, ShapeBounds> shapes_;
    std::unordered_map<uint16_t, Timeline> timelines_;
    std::vector<std::pair<std::string, uint16_t>> classes_;
    std::unordered_map<std::string, uint16_t> classIndex_;
    std::unordered_map<uint32_t, std::vector<FrameScriptOp>> scripts_; // (id << 16 | frame) -> statements
};

} // namespace engine
