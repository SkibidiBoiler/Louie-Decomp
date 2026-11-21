#pragma once
// AS: papaGame.models.effects.ItemGlowEffect — a glow behind another effect (whose id comes in the subtype): it fades
// in, pulses while following it, and fades out when told to stop.
#include "game/papaGame/models/Effect.h"

namespace papaGame::models::effects {

class ItemGlowEffect : public Effect {
public:
    using Effect::Effect;

    void init(double x, double y, double id, const std::string &subtype, bool followPlayer, double playerXoffset,
              double playerYoffset) override;
    void defineVars() override;
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void stopGlow();
    void updateObject() override;

    AnimationCycle fadeInCycleFrames = cycleType2("fadeIn", 1, 0, -1, {9, 8, 7, 6, 5});
    AnimationCycle fadeOutCycleFrames = cycleType2("fadeOut", 1, 0, -1, {5, 6, 7, 8, 9});
    AnimationCycle animCycleFrames = cycleType2("anim", 1, 0, 0, {0, 1, 2, 3, 4, 3, 2, 1});
    double targetEffectID = -1;
    bool fadingIn = true;
    bool fadingOut = false;
};

} // namespace papaGame::models::effects
