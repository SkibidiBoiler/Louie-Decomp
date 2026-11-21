#pragma once
// AS: papaGame.models.effects.PlayerStunSparkleEffect — a sparkle that twinkles at random spots around a stunned
// player's head until told to stop. (PL2 never adds it: the player's stun uses DizzyEffect.)
#include "game/papaGame/models/Effect.h"

namespace papaGame::models::effects {

class PlayerStunSparkleEffect : public Effect {
public:
    using Effect::Effect;

    void init(double x, double y, double id, const std::string &subtype, bool followPlayer, double playerXoffset,
              double playerYoffset) override;
    void defineVars() override;
    void changePlacement();
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void updateObject() override;
    void stopStun();

    AnimationCycle animCycleFrames = cycleType2("anim", 1, 0, -1, {0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1});
    AnimationCycle waitCycleFrames = cycleType2("wait", 1, 0, 0, {99});
    AnimationCycle endCycleFrames = cycleType2("end", 1, 0, -1, {3, 2, 1, 0});
    double waitTimer = 0;
    double waitTimerMax = 0;
    bool isEnding = false;
    double headCenterX = 0;
    double headCenterY = 0;
};

} // namespace papaGame::models::effects
