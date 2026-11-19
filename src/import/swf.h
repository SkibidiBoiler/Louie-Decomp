#pragma once
// SWF file reading for the in-app importer: decompression, tag iteration and the basic records.
// This only reads data to convert it to the port's own formats (docs/asset-format.md); nothing here plays SWF.
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace swfimport {

struct Rect {
    int32_t xmin = 0, xmax = 0, ymin = 0, ymax = 0; // twips
};

struct Matrix {
    float a = 1, b = 0, c = 0, d = 1, tx = 0, ty = 0; // tx/ty in pixels
};

struct CxForm {
    float mult[4] = {1, 1, 1, 1};
    float add[4] = {0, 0, 0, 0}; // 0..1
};

// MSB-first bit reader used by SWF records.
class Bits {
public:
    Bits(const uint8_t *data, size_t size, size_t pos) : d_(data), size_(size), byte_(pos) {}
    uint32_t ub(int n);
    int32_t sb(int n);
    float fb(int n) { return static_cast<float>(sb(n) / 65536.0); }
    size_t align();
    size_t pos() const { return byte_; }

private:
    const uint8_t *d_;
    size_t size_;
    size_t byte_;
    int bit_ = 0;
};

struct Tag {
    uint16_t code = 0;
    const uint8_t *body = nullptr;
    uint32_t size = 0;
};

class Swf {
public:
    // Reads and decompresses (FWS/CWS) a SWF; false with `error` set on failure.
    bool load(const std::vector<uint8_t> &file, std::string &error);

    const std::vector<uint8_t> &data() const { return data_; }
    uint8_t version() const { return version_; }
    Rect frameSize() const { return frame_; }
    double frameRate() const { return frameRate_; }
    uint16_t frameCount() const { return frameCount_; }
    size_t firstTag() const { return firstTag_; }

private:
    std::vector<uint8_t> data_; // whole uncompressed file
    uint8_t version_ = 0;
    Rect frame_;
    double frameRate_ = 0;
    uint16_t frameCount_ = 0;
    size_t firstTag_ = 0;
};

// Tags in data[pos, end): stops at End (code 0) or on a truncated header.
void forEachTag(const uint8_t *data, size_t pos, size_t end, const std::function<void(const Tag &)> &fn);

uint16_t u16(const uint8_t *p);
uint32_t u32(const uint8_t *p);
Rect readRect(const uint8_t *data, size_t size, size_t &pos);
Matrix readMatrix(const uint8_t *data, size_t size, size_t &pos);
CxForm readCxForm(const uint8_t *data, size_t size, size_t &pos, bool withAlpha);
std::string cstring(const uint8_t *data, size_t size, size_t &pos);

// zlib stream -> bytes (stb_image's inflater). Empty on error.
std::vector<uint8_t> inflate(const uint8_t *data, size_t size, size_t sizeHint = 0);

} // namespace swfimport
