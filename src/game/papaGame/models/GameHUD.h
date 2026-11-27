#pragma once
// AS: papaGame.models.GameHUD (§_-6Z§ on the game object) — the bar at the bottom: hearts, time, points, coins,
// Burgerzillas and special items, the MENU (pause) and sound buttons. SKELETON: the player portrait, the challenge
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
    void showTally(double /*tally*/, double /*target*/) {} // AS: GameHUD.showTally (empty in PL2)
    void updateAdjustmentNumbers() {}                      // AS: GameHUD.updateAdjustmentNumbers (empty)
    void updateTimer();
    void hideHUD();
    void showHUD();
    void updatePlayerHealth(double health);
    void clickMenu();
    void clickMute();
    void clickUnmute();
    void updateMuteButton();
    // Port: the HUD is drawn over the game in stage pixels (the original's display list does it).
    // With `viewExtraX` > 0 (widescreen) the screen is 700 + 2 * viewExtraX wide and starts viewExtraX left of the
    // stage: the bar's left part (MENU, hearts) keeps to the left edge, the rest (time ... sound) to the right
    // edge, and the plain bar between them is extended.
    void render(const engine::RenderContext &ctx, double viewExtraX = 0) const;
    // Port: the pointer (stage coordinates) in the HUD's own coordinates for that layout.
    engine::PointerState hudPointer(const engine::PointerState &pointer, double viewExtraX) const;

    obf_9G::obf_4R *gameObj = nullptr;
    std::unique_ptr<engine::MovieClip> interfaceClip; // AS: new hudMC()
};

} // namespace papaGame::models
