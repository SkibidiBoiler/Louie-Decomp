#pragma once
// Sound playback on SDL3 audio streams: the platform side of flash.media.Sound / SoundChannel / SoundMixer.
// Sounds are the importer's WAV files (sounds/<linkage name>.wav). Each playing sound has its own stream bound to
// the device (SDL mixes them); a stream callback feeds the samples with the channel's volume and pan applied.
#include <SDL3/SDL.h>

#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace engine {

struct SoundData {
    int rate = 44100;
    int channels = 1;
    std::vector<float> samples; // interleaved
    size_t frames() const { return samples.size() / static_cast<size_t>(channels); }
};

// A playing sound (flash.media.SoundChannel). A default-constructed channel is silent, like `new SoundChannel()`.
class SoundChannel {
public:
    SoundChannel() = default;
    ~SoundChannel();
    SoundChannel(const SoundChannel &) = delete;
    SoundChannel &operator=(const SoundChannel &) = delete;

    void stop();
    // soundTransform (volume 0..1, pan -1..1).
    double volume() const { return volume_.load(); }
    double pan() const { return pan_.load(); }
    void setTransform(double volume, double pan);
    bool playing() const { return stream_ != nullptr && !finished_.load(); }

private:
    friend class Audio;
    static void SDLCALL feed(void *userdata, SDL_AudioStream *stream, int additional, int total);

    SDL_AudioStream *stream_ = nullptr;
    std::shared_ptr<const SoundData> data_;
    size_t frame_ = 0;   // next frame to feed (audio thread)
    int loopsLeft_ = 0;  // replays after the first (audio thread)
    bool forever_ = false;
    std::atomic<double> volume_{1.0};
    std::atomic<double> pan_{0.0};
    std::atomic<bool> finished_{false};
    const std::atomic<double> *master_ = nullptr;
    std::vector<float> buffer_;
};

class Audio {
public:
    ~Audio();
    bool open();
    // Sounds are cached by name, so another game's folder drops the cache.
    void setSoundDir(const std::string &dir)
    {
        if (dir != dir_) {
            cache_.clear();
        }
        dir_ = dir;
    }
    // Sound.play(0, loops, transform). Null when the sound does not exist (getDefinitionByName fails).
    std::shared_ptr<SoundChannel> play(const std::string &name, int loops, double volume, double pan);
    // SoundMixer.soundTransform.volume
    void setMasterVolume(double v) { master_.store(v); }
    double masterVolume() const { return master_.load(); }
    // Drops the finished channels nobody holds any more.
    void update();
    // DEV: names started since the last call (--trace).
    std::vector<std::string> takeStarted();

private:
    std::shared_ptr<const SoundData> load(const std::string &name);

    SDL_AudioDeviceID device_ = 0;
    SDL_AudioSpec deviceSpec_{};
    std::string dir_;
    std::map<std::string, std::shared_ptr<const SoundData>> cache_; // null: missing
    std::vector<std::shared_ptr<SoundChannel>> active_;
    std::atomic<double> master_{1.0};
    std::vector<std::string> started_;
};

} // namespace engine
