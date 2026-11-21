#pragma once
// AS: papaGame.models.effects.SauceSplatEffect — the splash of a sauce shot.
#include "game/papaGame/models/Effect.h"

namespace papaGame::models::effects {

class SauceSplatEffect : public Effect {
public:
    using Effect::Effect;

    void defineVars() override;
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void updateObject() override;
};

} // namespace papaGame::models::effects
