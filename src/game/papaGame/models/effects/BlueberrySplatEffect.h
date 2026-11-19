#pragma once
// AS: papaGame.models.effects.BlueberrySplatEffect — the splash of a burst blueberry.
#include "game/papaGame/models/Effect.h"

namespace papaGame::models::effects {

class BlueberrySplatEffect : public Effect {
public:
    using Effect::Effect;

    void defineVars() override;
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void updateObject() override;
};

} // namespace papaGame::models::effects
