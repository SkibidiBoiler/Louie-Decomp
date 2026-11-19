#pragma once
// AS: papaGame.models.effects.ChiliSplatEffect — the splash of a burst chili can.
#include "game/papaGame/models/Effect.h"

namespace papaGame::models::effects {

class ChiliSplatEffect : public Effect {
public:
    using Effect::Effect;

    void defineVars() override;
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void updateObject() override;
};

} // namespace papaGame::models::effects
