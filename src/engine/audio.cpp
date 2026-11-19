#include "engine/audio.h"

#include <algorithm>
#include <climits>

namespace engine {

SoundChannel::~SoundChannel()
{
    stop();
}

void SoundChannel::stop()
{
    if (stream_) {
        SDL_DestroyAudioStream(stream_); // waits for a running callback
        stream_ = nullptr;
    }
    finished_.store(true);
}

void SoundChannel::setTransform(double volume, double pan)
{
    volume_.store(volume);
    pan_.store(pan);
}

// Called by SDL on the audio thread with the stream locked. `additional` is in bytes of the stream's input
// format (float, the sound's channels... always fed as stereo here).
void SDLCALL SoundChannel::feed(void *userdata, SDL_AudioStream *stream, int additional, int /*total*/)
{
    auto *ch = static_cast<SoundChannel *>(userdata);
    if (!ch->data_ || ch->finished_.load() || additional <= 0) {
        return;
    }
    const SoundData &d = *ch->data_;
    const size_t frames = d.frames();
    const size_t want = static_cast<size_t>(additional) / (2 * sizeof(float));
    ch->buffer_.resize(want * 2);
    const double vol = std::clamp(ch->volume_.load(), 0.0, 1.0) * (ch->master_ ? ch->master_->load() : 1.0);
    const double pan = ch->pan_.load();
    // SoundTransform pan: the far side is attenuated, the near side keeps full level.
    const float left = static_cast<float>(vol * (pan > 0 ? 1.0 - pan : 1.0));
    const float right = static_cast<float>(vol * (pan < 0 ? 1.0 + pan : 1.0));
    size_t out = 0;
    while (out < want) {
        if (ch->frame_ >= frames) {
            if (ch->forever_ || ch->loopsLeft_ > 0) {
                if (!ch->forever_) {
                    --ch->loopsLeft_;
                }
                ch->frame_ = 0;
                if (frames == 0) {
                    break;
                }
            } else {
                ch->finished_.store(true);
                break;
            }
        }
        const size_t n = std::min(want - out, frames - ch->frame_);
        for (size_t i = 0; i < n; ++i) {
            const size_t f = ch->frame_ + i;
            float l, r;
            if (d.channels == 1) {
                l = r = d.samples[f];
            } else {
                l = d.samples[2 * f];
                r = d.samples[2 * f + 1];
            }
            ch->buffer_[2 * (out + i)] = l * left;
            ch->buffer_[2 * (out + i) + 1] = r * right;
        }
        out += n;
        ch->frame_ += n;
    }
    if (out > 0) {
        SDL_PutAudioStreamData(stream, ch->buffer_.data(), static_cast<int>(out * 2 * sizeof(float)));
    }
}

Audio::~Audio()
{
    active_.clear();
    if (device_) {
        SDL_CloseAudioDevice(device_);
    }
}

bool Audio::open()
{
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        SDL_Log("Audio: %s", SDL_GetError());
        return false;
    }
    SDL_AudioSpec want{SDL_AUDIO_F32, 2, 44100};
    device_ = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &want);
    if (!device_) {
        SDL_Log("Audio: no playback device (%s)", SDL_GetError());
        return false;
    }
    int frames = 0;
    if (!SDL_GetAudioDeviceFormat(device_, &deviceSpec_, &frames)) {
        deviceSpec_ = want;
    }
    return true;
}

std::shared_ptr<const SoundData> Audio::load(const std::string &name)
{
    auto it = cache_.find(name);
    if (it != cache_.end()) {
        return it->second;
    }
    std::shared_ptr<SoundData> data;
    const std::string path = dir_ + "/" + name + ".wav";
    SDL_AudioSpec spec;
    Uint8 *buf = nullptr;
    Uint32 len = 0;
    if (SDL_LoadWAV(path.c_str(), &spec, &buf, &len)) {
        data = std::make_shared<SoundData>();
        data->rate = spec.freq;
        data->channels = std::clamp(spec.channels, 1, 2);
        if (spec.format == SDL_AUDIO_S16) {
            const auto *s = reinterpret_cast<const int16_t *>(buf);
            const size_t n = len / 2;
            data->samples.resize(n);
            for (size_t i = 0; i < n; ++i) {
                data->samples[i] = static_cast<float>(s[i]) / 32768.0f;
            }
        }
        SDL_free(buf);
    }
    cache_[name] = data;
    return data;
}

std::shared_ptr<SoundChannel> Audio::play(const std::string &name, int loops, double volume, double pan)
{
    std::shared_ptr<const SoundData> data = load(name);
    if (!data) {
        return nullptr;
    }
    auto ch = std::make_shared<SoundChannel>();
    ch->data_ = data;
    ch->forever_ = loops >= INT_MAX / 2;
    ch->loopsLeft_ = loops;
    ch->master_ = &master_;
    ch->setTransform(volume, pan);
    started_.push_back(name);
    if (device_) {
        const SDL_AudioSpec src{SDL_AUDIO_F32, 2, data->rate};
        ch->stream_ = SDL_CreateAudioStream(&src, &deviceSpec_);
        if (ch->stream_) {
            SDL_SetAudioStreamGetCallback(ch->stream_, SoundChannel::feed, ch.get());
            if (!SDL_BindAudioStream(device_, ch->stream_)) {
                SDL_DestroyAudioStream(ch->stream_);
                ch->stream_ = nullptr;
            }
        }
    }
    if (!ch->stream_) {
        ch->finished_.store(true); // no device: the channel exists but stays silent
    }
    active_.push_back(ch);
    return ch;
}

void Audio::update()
{
    active_.erase(std::remove_if(active_.begin(), active_.end(),
                                 [](const std::shared_ptr<SoundChannel> &c) {
                                     return !c->playing() && c.use_count() == 1;
                                 }),
                  active_.end());
    // Finished channels still held by the game: release their streams early.
    for (auto &c : active_) {
        if (c->stream_ && c->finished_.load() && SDL_GetAudioStreamQueued(c->stream_) == 0) {
            c->stop();
        }
    }
}

std::vector<std::string> Audio::takeStarted()
{
    std::vector<std::string> out;
    out.swap(started_);
    return out;
}

} // namespace engine
