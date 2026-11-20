#pragma once
// AS: papaGame.models.effects.CageCustomerEffect — the customer trapped in a cage (GameObject19), drawn from the
// caged sheet BitmapManager blitted for that cage: it struggles, stands up when the cage opens, jumps with the
// player and flies into the end portal.
#include "game/papaGame/models/Effect.h"

namespace papaGame::models::effects {

class CageCustomerEffect : public Effect {
public:
    using Effect::Effect;

    void init(double x, double y, double id, const std::string &subtype, bool followPlayer, double playerXoffset,
              double playerYoffset) override;
    void defineVars() override;
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void updateObject() override;
    void openCage() { isOpen = true; } // AS: CageCustomerEffect.openCage
    bool checkOnScreen() const override;
    const engine::Sheet *getSheet() const override;

    AnimationCycle closedCycleFrames =
        cycleType2("closed", 1, 0, 0, {0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3});
    AnimationCycle openCycleFrames = cycleType2("open", 1, 0, -1, {0, 4, 5, 6});
    AnimationCycle jumpCycleFrames = cycleType2("jump", 1, 0, 2, {7, 8, 9});
    AnimationCycle fallCycleFrames = cycleType2("fall", 1, 0, 2, {10, 11, 12});
    AnimationCycle standCycleFrames =
        cycleType2("stand", 1, 0, 0, {13, 13, 13, 13, 14, 14, 14, 14, 15, 15, 15, 15, 14, 14, 14, 14});
    AnimationCycle goneCycleFrames = cycleType2("gone", 1, 0, 0, {999});
    bool isOpen = false;
    bool hasStoodUp = false;
    bool isGone = false;
    double portalX = -1;
    double portalY = -1;
};

} // namespace papaGame::models::effects
