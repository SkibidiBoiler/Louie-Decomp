#pragma once
// AS: papaGame.managers.SoundManager (§_-7C§ on the game object) — sound effects and the two music channels
// that cross-fade between tracks.
#include <memory>
#include <string>

#include "engine/audio.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::managers {

class SoundManager {
public:
    static inline const std::string FADE_NONE = "none";
    static inline const std::string FADE_CROSSFADE = "crossfade";
    static inline const std::string FADE_OUTIN = "outin";

    SoundManager(obf_9G::obf_4R *gameObj, engine::Audio *audio);

    void setupEngine();
    void startLevelMusic();
    void playTrack(const std::string &trackName, double loop = 1, double offset = 0,
                   const std::string &fadeType = "none");
    void switchMusic(double useTrack);
    void fadeMusic();
    void endMusic();
    void pauseMusic(bool paused);
    void toggleMute();
    void muteSound(bool permanent = false);
    void unmuteSound(bool permanent = false);
    std::shared_ptr<engine::SoundChannel> playSound(const std::string &which, bool loop = false,
                                                    double targetVolume = -1, double targetPan = 0);
    // Port: drives fadeTimer (a flash.utils.Timer of fadeSpeed ms) from the game's clock.
    void advanceTime(double ms);

    obf_9G::obf_4R *gameObj = nullptr;
    bool isMute = false;
    double maxvolume = 1;
    double lastVolume = 100;
    bool fadeTimer = false; // port: the Timer exists (started) while true
    double fadeTimerElapsed = 0;
    double fadeSpeed = 100;
    double fadeAmount = 0.05;
    std::string currentTrack;
    std::shared_ptr<engine::SoundChannel> musicTrack1;
    std::shared_ptr<engine::SoundChannel> musicTrack2;
    double currentTrackNumber = 1;
    std::string currentFadeType = "none";
    std::string musicMode = "level";
    bool hasShownTitleScreen = false;

private:
    engine::Audio *audio_ = nullptr; // port: the platform mixer
};

} // namespace papaGame::managers
