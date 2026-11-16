#pragma once
// Converts the SWF's DefineSound tags to WAV files: sounds/<AS3 linkage name>.wav (docs/asset-format.md).
// MP3 is decoded with minimp3; uncompressed PCM is copied; ADPCM is not handled (PL2 has none).
#include <string>

#include "import/symbols_export.h"

namespace swfimport {

// Writes one WAV per sound that has an AS3 class name. Returns how many were written; `skipped` counts the
// sounds that could not be converted.
size_t exportSounds(const SwfIndex &index, const std::string &outDir, size_t &skipped);

} // namespace swfimport
