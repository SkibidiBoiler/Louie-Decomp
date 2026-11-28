#pragma once
// Converts the character table, shape bounds, class names and sprite timelines of a SWF into symbols.bin
// (docs/asset-format.md). Same output as tools/swf2assets.py.
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "import/swf.h"

namespace swfimport {

enum Kind : uint8_t { KindShape = 1, KindSprite, KindMorph, KindText, KindEditText, KindButton, KindBitmap, KindSound, KindFont };

struct SwfIndex {
    std::map<uint16_t, uint8_t> chars;               // id -> Kind
    std::map<uint16_t, Tag> defs;                    // id -> defining tag (shapes, bitmaps, sounds, ...)
    std::map<std::string, uint16_t> classes;         // AS3 linkage name -> id
    std::vector<Tag> abc;                            // DoABC / DoABC2 tags
};

struct Key {
    uint16_t frame = 0;
    Matrix matrix;
    std::optional<CxForm> cx;
    std::optional<uint16_t> ratio;
};

struct TrackData {
    uint16_t depth = 0, charId = 0;
    std::string name;
    uint16_t clipDepth = 0;
    uint8_t blend = 0, filters = 0;
    int first = 0, last = -1;
    Matrix matrix;
    std::optional<CxForm> cx;
    std::optional<uint16_t> ratio;
    std::vector<Key> keys;
};

struct TimelineData {
    uint16_t frameCount = 0;
    std::vector<std::pair<std::string, uint16_t>> labels;
    std::vector<std::unique_ptr<TrackData>> tracks;
};

// Timeline of a sprite (or of the main movie) in data[pos, end).
TimelineData parseTimeline(const uint8_t *data, size_t pos, size_t end, uint16_t frameCount);

// Walks the top-level tags once.
SwfIndex indexSwf(const Swf &swf);

// Every (morph shape id, ratio) the timelines place, for the importer to draw each step.
std::vector<std::pair<uint16_t, uint16_t>> morphUses(const Swf &swf, const SwfIndex &index);

// Builds symbols.bin in memory.
std::vector<uint8_t> buildSymbols(const Swf &swf, const SwfIndex &index);

} // namespace swfimport
