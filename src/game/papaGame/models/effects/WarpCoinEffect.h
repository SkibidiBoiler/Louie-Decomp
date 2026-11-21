#pragma once
// AS: papaGame.models.effects.WarpCoinEffect — the warp key earned from a level: it rises over the player's head with
// a glow, spins, and goes when its animation ends.
#include "game/papaGame/models/Effect.h"

namespace papaGame::models::effects {

class WarpCoinEffect : public Effect {
public:
    using Effect::Effect;

    void init(double x, double y, double id, const std::string &subtype, bool followPlayer, double playerXoffset,
              double playerYoffset) override;
    void defineVars() override;
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void updateObject() override;
    void destroy() override;

    AnimationCycle animCycleFrames = cycleType2(
        "anim", 1, 0, -1, {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 14, 15, 15, 16, 16,
                           16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
                           16, 15, 14, 13, 12, 11, 10, 9,  8,  7,  6,  5,  4,  3,  2,  1,  0});
    double targetY = 80;
    double glowEffectID = -1;
};

} // namespace papaGame::models::effects
