#pragma once
// AS: papaGame.models.effects.DizzyEffect — the dizzy stars over a stunned enemy or player.
#include "game/papaGame/models/Effect.h"

namespace papaGame::models::effects {

class DizzyEffect : public Effect {
public:
    using Effect::Effect;

    void defineVars() override;
    const AnimationCycle &grabAnimationCycle() override;
    void updateObject() override;
    void stopStun();
};

} // namespace papaGame::models::effects
