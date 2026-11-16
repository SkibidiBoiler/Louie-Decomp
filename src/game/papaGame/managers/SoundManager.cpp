#include "game/papaGame/managers/SoundManager.h"

#include <SDL3/SDL.h>

#include <climits>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::managers {

SoundManager::SoundManager(obf_9G::obf_4R *gameObj_, engine::Audio *audio) : gameObj(gameObj_), audio_(audio)
{
    setupEngine();
}

// AS: SoundManager.setupEngine — the Flipline API's mute events are not ported (the API is dropped).
void SoundManager::setupEngine()
{
    musicTrack1 = std::make_shared<engine::SoundChannel>();
    musicTrack2 = std::make_shared<engine::SoundChannel>();
}

// AS: SoundManager.startLevelMusic
void SoundManager::startLevelMusic()
{
    const double level = gameObj->obf_AQ->currentLevel;
    if (level == 0 || level == 1) {
        playTrack("LettuceTrack", INT_MAX);
    } else if (level == 2 || level == 3) {
        playTrack("OnionTrack", INT_MAX);
    } else if (level == 4 || level == 5) {
        playTrack("CheeseTrack", INT_MAX);
    } else if (level == 8) {
        playTrack("PreBossTrack", INT_MAX);
    } else if (level == 9) {
        playTrack("XZoneTrack", INT_MAX);
    } else {
        playTrack("BBQTrack", INT_MAX);
    }
}

// AS: SoundManager.playTrack — the new track goes on the idle channel; `loop` and `offset` are not used.
void SoundManager::playTrack(const std::string &trackName, double /*loop*/, double /*offset*/,
                             const std::string &fadeType)
{
    if (currentTrack == trackName) {
        return;
    }
    const double useTrack = currentTrackNumber == 1 ? 2 : 1;
    currentTrack = trackName;
    currentFadeType = fadeType;
    const double volume = fadeType != FADE_NONE ? 0.1 : 1;
    std::shared_ptr<engine::SoundChannel> &channel = useTrack == 1 ? musicTrack1 : musicTrack2;
    if (channel) {
        channel->stop();
    }
    if (!trackName.empty()) {
        // getDefinitionByName fails for a missing track: the old (stopped) channel stays.
        if (std::shared_ptr<engine::SoundChannel> played = audio_ ? audio_->play(trackName, INT_MAX, 1, 0) : nullptr) {
            channel = played;
        }
    }
    if (channel) {
        channel->setTransform(volume, 0);
    }
    if (fadeType != FADE_NONE) {
        switchMusic(useTrack);
    } else if (useTrack == 1) {
        if (musicTrack2) {
            musicTrack2->stop();
        }
    } else if (musicTrack1) {
        musicTrack1->stop();
    }
}

// AS: SoundManager.switchMusic
void SoundManager::switchMusic(double useTrack)
{
    if (!isMute) {
        if (useTrack != currentTrackNumber) {
            currentTrackNumber = useTrack;
            if (!fadeTimer) {
                fadeTimer = true;
                fadeTimerElapsed = 0;
            }
        }
    } else {
        currentTrackNumber = useTrack;
    }
}

// AS: SoundManager.fadeMusic (the fadeTimer's TIMER handler)
void SoundManager::fadeMusic()
{
    double vol1 = 1, vol2 = 1; // new SoundTransform(): volume 1
    const double cur1 = musicTrack1 ? musicTrack1->volume() : 0;
    const double cur2 = musicTrack2 ? musicTrack2->volume() : 0;
    if (currentFadeType == FADE_CROSSFADE) {
        if (currentTrackNumber == 1) {
            vol1 = cur1 + fadeAmount;
            vol2 = cur2 - fadeAmount;
        } else if (currentTrackNumber == 2) {
            vol1 = cur1 - fadeAmount;
            vol2 = cur2 + fadeAmount;
        }
    } else if (currentFadeType == FADE_OUTIN) {
        if (currentTrackNumber == 1) {
            if (musicTrack2 && cur2 <= 0.4) {
                if (musicTrack1) {
                    vol1 = cur1 + fadeAmount * 2;
                }
            } else {
                vol1 = 0;
            }
            if (musicTrack2 && cur2 > 0) {
                vol2 = cur2 - fadeAmount * 2;
            } else {
                vol2 = 0;
            }
        } else if (currentTrackNumber == 2) {
            if (musicTrack1 && cur1 > 0) {
                vol1 = cur1 - fadeAmount * 2;
            } else {
                vol1 = 0;
            }
            if (musicTrack1 && cur1 <= 0.4) {
                if (musicTrack2) {
                    vol2 = cur2 + fadeAmount * 2;
                }
            } else {
                vol2 = 0;
            }
        }
    } else if (currentFadeType == FADE_NONE) {
        if (currentTrackNumber == 1) {
            vol1 = maxvolume;
            vol2 = 0;
        } else if (currentTrackNumber == 2) {
            vol1 = 0;
            vol2 = maxvolume;
        }
    }
    bool done = false;
    if (vol1 >= maxvolume && currentTrackNumber == 1) {
        vol1 = maxvolume;
        vol2 = 0;
        done = true;
    } else if (vol2 >= maxvolume && currentTrackNumber == 2) {
        vol1 = 0;
        vol2 = maxvolume;
        done = true;
    }
    if (done && fadeTimer) {
        fadeTimer = false;
    }
    if (isMute) {
        vol1 = 0;
        vol2 = 0;
    }
    if (musicTrack1) {
        musicTrack1->setTransform(vol1, 0);
    }
    if (musicTrack2) {
        musicTrack2->setTransform(vol2, 0);
    }
}

// AS: SoundManager.endMusic
void SoundManager::endMusic()
{
    if (musicTrack1) {
        musicTrack1->stop();
    }
    if (musicTrack2) {
        musicTrack2->stop();
    }
    currentTrack = "none";
}

// AS: SoundManager.pauseMusic
void SoundManager::pauseMusic(bool paused)
{
    if (!audio_) {
        return;
    }
    if (paused) {
        lastVolume = audio_->masterVolume();
        audio_->setMasterVolume(0);
    } else {
        audio_->setMasterVolume(lastVolume);
    }
}

// AS: SoundManager.toggleMute
void SoundManager::toggleMute()
{
    isMute = !isMute;
    if (audio_) {
        audio_->setMasterVolume(isMute ? 0 : maxvolume);
    }
}

// AS: SoundManager.muteSound — the "soundIsMuted" event has no listener in the game.
void SoundManager::muteSound(bool permanent)
{
    if (permanent) {
        isMute = true;
    }
    if (audio_) {
        audio_->setMasterVolume(0);
    }
}

// AS: SoundManager.unmuteSound — the "soundIsNotMuted" event has no listener in the game.
void SoundManager::unmuteSound(bool permanent)
{
    if (permanent) {
        isMute = false;
    }
    if (audio_) {
        audio_->setMasterVolume(maxvolume);
    }
}

// AS: SoundManager.playSound
std::shared_ptr<engine::SoundChannel> SoundManager::playSound(const std::string &which, bool loop,
                                                              double targetVolume, double targetPan)
{
    double volume;
    if (isMute) {
        volume = 0;
    } else if (targetVolume == -1) {
        volume = maxvolume;
    } else {
        volume = targetVolume;
    }
    std::shared_ptr<engine::SoundChannel> channel =
        audio_ ? audio_->play(which, loop ? INT_MAX : 0, volume, targetPan) : nullptr;
    if (!channel) {
        SDL_Log("Error Playing Sound: %s.", which.c_str());
    }
    return channel;
}

void SoundManager::advanceTime(double ms)
{
    if (!fadeTimer) {
        return;
    }
    fadeTimerElapsed += ms;
    while (fadeTimer && fadeTimerElapsed >= fadeSpeed) {
        fadeTimerElapsed -= fadeSpeed;
        fadeMusic();
    }
}

} // namespace papaGame::managers
