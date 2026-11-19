#pragma once
// AS: papaGame.models.effects.DoorFrontEffect — the front of a door, drawn over the player as they pass through.
#include "game/papaGame/models/Effect.h"

namespace papaGame::models::effects {

class DoorFrontEffect : public Effect {
public:
    using Effect::Effect;

    void defineVars() override;
    const AnimationCycle &grabAnimationCycle() override;
    void updateObject() override;
    bool checkOnScreen() const override;
};

} // namespace papaGame::models::effects
