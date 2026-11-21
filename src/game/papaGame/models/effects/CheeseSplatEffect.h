#pragma once
// AS: papaGame.models.effects.CheeseSplatEffect — the splash of a cheese shot.
#include "game/papaGame/models/Effect.h"

namespace papaGame::models::effects {

class CheeseSplatEffect : public Effect {
public:
    using Effect::Effect;

    void defineVars() override;
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void updateObject() override;
};

} // namespace papaGame::models::effects
