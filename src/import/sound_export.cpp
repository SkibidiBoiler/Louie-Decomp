#include "import/sound_export.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cstring>
#include <vector>

#if defined(_MSC_VER)
#pragma warning(push, 0)
#elif defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
#define MINIMP3_IMPLEMENTATION
#include "minimp3.h"
#if defined(_MSC_VER)
#pragma warning(pop)
#elif defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

namespace swfimport {

namespace {

void put16(std::vector<uint8_t> &out, uint32_t v)
{
    out.push_back(static_cast<uint8_t>(v));
    out.push_back(static_cast<uint8_t>(v >> 8));
}

void put32(std::vector<uint8_t> &out, uint32_t v)
{
    put16(out, v & 0xFFFF);
    put16(out, v >> 16);
}

// 16-bit PCM WAV (RIFF) around interleaved samples.
std::vector<uint8_t> wavFile(const std::vector<int16_t> &samples, int rate, int channels)
{
    const uint32_t dataBytes = static_cast<uint32_t>(samples.size() * 2);
    std::vector<uint8_t> out;
    out.reserve(44 + dataBytes);
    out.insert(out.end(), {'R', 'I', 'F', 'F'});
    put32(out, 36 + dataBytes);
    out.insert(out.end(), {'W', 'A', 'V', 'E', 'f', 'm', 't', ' '});
    put32(out, 16);
    put16(out, 1); // PCM
    put16(out, static_cast<uint32_t>(channels));
    put32(out, static_cast<uint32_t>(rate));
    put32(out, static_cast<uint32_t>(rate * channels * 2));
    put16(out, static_cast<uint32_t>(channels * 2));
    put16(out, 16);
    out.insert(out.end(), {'d', 'a', 't', 'a'});
    put32(out, dataBytes);
    for (int16_t s : samples) {
        put16(out, static_cast<uint16_t>(s));
    }
    return out;
}

// MP3SOUNDDATA: SeekSamples (SI16) then MP3 frames. Flash skips SeekSamples samples at the start (the encoder
// delay) and plays SampleCount samples, which keeps looping music seamless.
bool decodeMp3(const uint8_t *data, size_t size, uint32_t sampleCount, std::vector<int16_t> &out, int &rate,
               int &channels)
{
    if (size < 2) {
        return false;
    }
    const int seek = static_cast<int16_t>(data[0] | (data[1] << 8));
    mp3dec_t dec;
    mp3dec_init(&dec);
    mp3d_sample_t pcm[MINIMP3_MAX_SAMPLES_PER_FRAME];
    size_t pos = 2;
    rate = 0;
    channels = 0;
    while (pos < size) {
        mp3dec_frame_info_t info;
        const int n = mp3dec_decode_frame(&dec, data + pos, static_cast<int>(size - pos), pcm, &info);
        if (info.frame_bytes == 0) {
            break;
        }
        pos += static_cast<size_t>(info.frame_bytes);
        if (n == 0) {
            continue;
        }
        if (channels == 0) {
            channels = info.channels;
            rate = info.hz;
        }
        if (info.channels != channels) {
            continue; // a stray frame in the other channel layout
        }
        out.insert(out.end(), pcm, pcm + n * channels);
    }
    if (channels == 0) {
        return false;
    }
    const size_t skip = seek > 0 ? static_cast<size_t>(seek) * static_cast<size_t>(channels) : 0;
    out.erase(out.begin(), out.begin() + static_cast<std::ptrdiff_t>(std::min(skip, out.size())));
    const size_t keep = static_cast<size_t>(sampleCount) * static_cast<size_t>(channels);
    if (sampleCount > 0 && out.size() > keep) {
        out.resize(keep);
    }
    return true;
}

} // namespace

size_t exportSounds(const SwfIndex &index, const std::string &outDir, size_t &skipped)
{
    skipped = 0;
    const std::string dir = outDir + "/sounds";
    SDL_CreateDirectory(dir.c_str());
    size_t written = 0;
    for (const auto &[name, id] : index.classes) {
        auto def = index.defs.find(id);
        if (def == index.defs.end() || def->second.code != 14) {
            continue;
        }
        const Tag &tag = def->second;
        if (tag.size < 7) {
            ++skipped;
            continue;
        }
        // DefineSound: SoundId, SoundFormat:4 SoundRate:2 SoundSize:1 SoundType:1, SoundSampleCount, SoundData.
        const uint8_t flags = tag.body[2];
        const int format = flags >> 4;
        static const int kRates[] = {5512, 11025, 22050, 44100};
        int rate = kRates[(flags >> 2) & 3];
        const bool is16 = (flags & 2) != 0;
        int channels = (flags & 1) ? 2 : 1;
        const uint32_t sampleCount = u32(tag.body + 3);
        const uint8_t *data = tag.body + 7;
        const size_t size = tag.size - 7;
        std::vector<int16_t> samples;
        bool ok = false;
        if (format == 2) {
            ok = decodeMp3(data, size, sampleCount, samples, rate, channels);
        } else if (format == 0 || format == 3) {
            // Uncompressed: format 3 is little-endian; format 0 is native order, little-endian in practice.
            if (is16) {
                samples.resize(size / 2);
                for (size_t i = 0; i < samples.size(); ++i) {
                    samples[i] = static_cast<int16_t>(data[2 * i] | (data[2 * i + 1] << 8));
                }
            } else {
                samples.resize(size);
                for (size_t i = 0; i < size; ++i) {
                    samples[i] = static_cast<int16_t>((data[i] - 128) << 8);
                }
            }
            ok = true;
        }
        if (!ok || samples.empty()) {
            SDL_Log("Import: sound %s not converted (format %d)", name.c_str(), format);
            ++skipped;
            continue;
        }
        const std::vector<uint8_t> wav = wavFile(samples, rate, channels);
        const std::string path = dir + "/" + name + ".wav";
        SDL_IOStream *io = SDL_IOFromFile(path.c_str(), "wb");
        if (!io) {
            ++skipped;
            continue;
        }
        const bool done = SDL_WriteIO(io, wav.data(), wav.size()) == wav.size();
        SDL_CloseIO(io);
        if (done) {
            ++written;
        } else {
            ++skipped;
        }
    }
    return written;
}

} // namespace swfimport
