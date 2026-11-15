#pragma once
// SHA-256 (FIPS 180-4), used to recognise the owner's copy of each game.
#include <cstddef>
#include <cstdint>
#include <string>

namespace swfimport {

std::string sha256Hex(const uint8_t *data, size_t size);

} // namespace swfimport
