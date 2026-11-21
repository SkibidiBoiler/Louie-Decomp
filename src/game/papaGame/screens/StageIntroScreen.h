#pragma once
// AS: papaGame.screens.StageIntroScreen — the spinning intro before a level, with the chosen customer falling
// in; it starts the level (§_-6V§) when the player's sprite sheets are ready.
#include <memory>

#include "engine/audio.h"
#include "engine/movieclip.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::screens {

class StageIntroScreen {
public:
    explicit StageIntroScreen(obf_9G::obf_4R *gameObj);

    void setupScreen();
    void updateScreen();
    void spriteReady();
    void cagedSpritesReady();
    void destroy();

    obf_9G::obf_4R *gameObj = nullptr;
    engine::MovieClip *clip = nullptr; // owned by the game's container
    double delayTimer = 0;
    double startFadeTimer = 20;
    double delayTimerMax = 60;
    bool playerReady = false;
    bool cagesReady = false;
    bool gameReady = true;
    engine::MovieClip *playerMC = nullptr; // owned by clip.inside
    // Port: the intro's "startofday.wav", held so the intro can wait for it (option waitIntroMusic), and the frames
    // waited on the last frame (capped at kMaxMusicWait).
    std::shared_ptr<engine::SoundChannel> introSound;
    int musicWait_ = 0;
    static constexpr int kMaxMusicWait = 150;

private:
    std::unique_ptr<engine::MovieClip> buildModel(double whichCharacter, double style);
};

} // namespace papaGame::screens
