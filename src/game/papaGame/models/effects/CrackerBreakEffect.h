#pragma once
// AS: papaGame.models.effects.CrackerBreakEffect — a cracker crumb flying off a block.
#include "game/papaGame/models/Effect.h"

namespace papaGame::models::effects {

class CrackerBreakEffect : public Effect {
public:
    using Effect::Effect;

    void init(double x, double y, double id, const std::string &subtype, bool followPlayer, double playerXoffset,
              double playerYoffset) override;
    void defineVars() override;
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void updateObject() override;
};

} // namespace papaGame::models::effects
