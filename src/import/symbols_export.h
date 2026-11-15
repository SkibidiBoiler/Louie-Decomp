#pragma once
// Converts the character table, shape bounds, class names and sprite timelines of a SWF into symbols.bin
// (docs/asset-format.md). Same output as tools/swf2assets.py.
#include <cstdint>
#include <map>
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

// Walks the top-level tags once.
SwfIndex indexSwf(const Swf &swf);

// Builds symbols.bin in memory.
std::vector<uint8_t> buildSymbols(const Swf &swf, const SwfIndex &index);

} // namespace swfimport
