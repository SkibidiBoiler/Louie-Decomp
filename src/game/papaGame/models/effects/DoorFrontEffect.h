#pragma once
// AS: papaGame.models.effects.DoorFrontEffect — the front frame of a door (GameObject18), which the door moves
// in front of or behind the player.
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
