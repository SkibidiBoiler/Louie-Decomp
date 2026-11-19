#pragma once
// AS: papaGame.models.effects.ItemFizzoCanEffect — the collected FizzoCan jumping up over the player and back down.
#include "game/papaGame/models/Effect.h"

namespace papaGame::models::effects {

class ItemFizzoCanEffect : public Effect {
public:
    using Effect::Effect;

    void defineVars() override;
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void updateObject() override;

    double distanceToJump = 65;
    double startingY = 0;
    double timePerDirection = 8;
    double jumpTimer = 0;
    bool goingUp = true;
};

} // namespace papaGame::models::effects
