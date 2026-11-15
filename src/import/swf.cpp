#include "import/swf.h"

#include <cstring>

#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#include "stb_image.h"

namespace swfimport {

uint16_t u16(const uint8_t *p)
{
    return static_cast<uint16_t>(p[0] | (p[1] << 8));
}

uint32_t u32(const uint8_t *p)
{
    return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) | (static_cast<uint32_t>(p[2]) << 16) |
           (static_cast<uint32_t>(p[3]) << 24);
}

uint32_t Bits::ub(int n)
{
    uint32_t v = 0;
    for (int i = 0; i < n; ++i) {
        const uint32_t bit = byte_ < size_ ? (d_[byte_] >> (7 - bit_)) & 1u : 0u;
        v = (v << 1) | bit;
        if (++bit_ == 8) {
            bit_ = 0;
            ++byte_;
        }
    }
    return v;
}

int32_t Bits::sb(int n)
{
    const uint32_t v = ub(n);
    if (n > 0 && n < 32 && (v & (1u << (n - 1)))) {
        return static_cast<int32_t>(v) - static_cast<int32_t>(1u << n);
    }
    return static_cast<int32_t>(v);
}

size_t Bits::align()
{
    if (bit_) {
        bit_ = 0;
        ++byte_;
    }
    return byte_;
}

std::vector<uint8_t> inflate(const uint8_t *data, size_t size, size_t sizeHint)
{
    int outLen = 0;
    char *out = stbi_zlib_decode_malloc_guesssize(reinterpret_cast<const char *>(data), static_cast<int>(size),
                                                  static_cast<int>(sizeHint ? sizeHint : size * 4), &outLen);
    if (!out) {
        return {};
    }
    std::vector<uint8_t> result(reinterpret_cast<uint8_t *>(out), reinterpret_cast<uint8_t *>(out) + outLen);
    STBI_FREE(out);
    return result;
}

bool Swf::load(const std::vector<uint8_t> &file, std::string &error)
{
    if (file.size() < 8 || file[1] != 'W' || file[2] != 'S') {
        error = "not a SWF file";
        return false;
    }
    version_ = file[3];
    const uint32_t length = u32(&file[4]);
    if (file[0] == 'F') {
        data_ = file;
    } else if (file[0] == 'C') {
        data_.assign(file.begin(), file.begin() + 8);
        const std::vector<uint8_t> body = inflate(file.data() + 8, file.size() - 8, length);
        if (body.empty()) {
            error = "could not decompress the SWF";
            return false;
        }
        data_.insert(data_.end(), body.begin(), body.end());
    } else {
        error = file[0] == 'Z' ? "LZMA-compressed SWF files are not supported yet" : "not a SWF file";
        return false;
    }
    size_t pos = 8;
    frame_ = readRect(data_.data(), data_.size(), pos);
    if (pos + 4 > data_.size()) {
        error = "truncated SWF header";
        return false;
    }
    frameRate_ = u16(&data_[pos]) / 256.0;
    frameCount_ = u16(&data_[pos + 2]);
    firstTag_ = pos + 4;
    return true;
}

void forEachTag(const uint8_t *data, size_t pos, size_t end, const std::function<void(const Tag &)> &fn)
{
    while (pos + 2 <= end) {
        const uint16_t header = u16(data + pos);
        pos += 2;
        Tag tag;
        tag.code = static_cast<uint16_t>(header >> 6);
        tag.size = header & 0x3F;
        if (tag.size == 0x3F) {
            if (pos + 4 > end) {
                return;
            }
            tag.size = u32(data + pos);
            pos += 4;
        }
        if (pos + tag.size > end) {
            return;
        }
        tag.body = data + pos;
        pos += tag.size;
        if (tag.code == 0) {
            return;
        }
        fn(tag);
    }
}

Rect readRect(const uint8_t *data, size_t size, size_t &pos)
{
    Bits b(data, size, pos);
    const int n = static_cast<int>(b.ub(5));
    Rect r;
    r.xmin = b.sb(n);
    r.xmax = b.sb(n);
    r.ymin = b.sb(n);
    r.ymax = b.sb(n);
    pos = b.align();
    return r;
}

Matrix readMatrix(const uint8_t *data, size_t size, size_t &pos)
{
    Bits b(data, size, pos);
    Matrix m;
    if (b.ub(1)) {
        const int n = static_cast<int>(b.ub(5));
        m.a = b.fb(n);
        m.d = b.fb(n);
    }
    if (b.ub(1)) {
        const int n = static_cast<int>(b.ub(5));
        m.b = b.fb(n);
        m.c = b.fb(n);
    }
    const int n = static_cast<int>(b.ub(5));
    m.tx = static_cast<float>(b.sb(n) / 20.0);
    m.ty = static_cast<float>(b.sb(n) / 20.0);
    pos = b.align();
    return m;
}

CxForm readCxForm(const uint8_t *data, size_t size, size_t &pos, bool withAlpha)
{
    Bits b(data, size, pos);
    const bool hasAdd = b.ub(1) != 0;
    const bool hasMult = b.ub(1) != 0;
    const int n = static_cast<int>(b.ub(4));
    const int count = withAlpha ? 4 : 3;
    CxForm cx;
    if (hasMult) {
        for (int i = 0; i < count; ++i) {
            cx.mult[i] = static_cast<float>(b.sb(n) / 256.0);
        }
    }
    if (hasAdd) {
        for (int i = 0; i < count; ++i) {
            cx.add[i] = static_cast<float>(b.sb(n) / 255.0);
        }
    }
    pos = b.align();
    return cx;
}

std::string cstring(const uint8_t *data, size_t size, size_t &pos)
{
    const size_t start = pos;
    while (pos < size && data[pos] != 0) {
        ++pos;
    }
    std::string s(reinterpret_cast<const char *>(data + start), pos - start);
    if (pos < size) {
        ++pos;
    }
    return s;
}

} // namespace swfimport
