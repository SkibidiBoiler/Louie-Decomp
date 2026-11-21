#pragma once
// AS: papaGame.models.effects.BossPropellerEffect — Radley Madish's spinning propeller; it follows the boss whose id
// comes in the subtype.
#include "game/papaGame/models/Effect.h"

namespace papaGame::models::effects {

class BossPropellerEffect : public Effect {
public:
    using Effect::Effect;

    void init(double x, double y, double id, const std::string &subtype, bool followPlayer, double playerXoffset,
              double playerYoffset) override;
    void defineVars() override;
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override {} // AS: BossPropellerEffect.endAnimationCycle (empty)
    void updateObject() override;
    bool checkOnScreen() const override { return true; } // AS: BossPropellerEffect.checkOnScreen

    double bossID = -1;
};

} // namespace papaGame::models::effects
