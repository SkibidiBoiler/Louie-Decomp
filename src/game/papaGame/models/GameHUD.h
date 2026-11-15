#pragma once
// AS: papaGame.models.GameHUD (§_-6Z§ on the game object) — the bar at the bottom: hearts, time, points, coins,
// Burgerzillas and special items. SKELETON: the menu / mute / pause buttons, the player portrait, the challenge
// bubbles and the FPS toggle.
#include <memory>

#include "engine/movieclip.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::models {

class GameHUD {
public:
    explicit GameHUD(obf_9G::obf_4R *gameObj);

    void setupHUD();
    void updateDisplay();
    void updateTimer();
    void hideHUD();
    void showHUD();
    void updatePlayerHealth(double health);
    // Port: the HUD is drawn over the game in stage pixels (the original's display list does it).
    void render(const engine::RenderContext &ctx) const;

    obf_9G::obf_4R *gameObj = nullptr;
    std::unique_ptr<engine::MovieClip> interfaceClip; // AS: new hudMC()
};

} // namespace papaGame::models
