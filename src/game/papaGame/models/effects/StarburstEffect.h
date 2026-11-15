#pragma once
// AS: papaGame.models.effects.StarburstEffect — the burst where an enemy is hit.
#include "game/papaGame/models/Effect.h"

namespace papaGame::models::effects {

class StarburstEffect : public Effect {
public:
    using Effect::Effect;

    void defineVars() override;
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void updateObject() override;
};

} // namespace papaGame::models::effects
