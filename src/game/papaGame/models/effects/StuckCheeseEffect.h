#pragma once
// AS: papaGame.models.effects.StuckCheeseEffect — the lump of cheese stuck on a cheesed player; each jump shakes a
// piece off, and when the last piece goes the player is free.
#include "game/papaGame/models/Effect.h"

namespace papaGame::models::effects {

class StuckCheeseEffect : public Effect {
public:
    using Effect::Effect;

    void defineVars() override;
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void updateObject() override;
    void playerJumped();
    void refillCheese();
    bool checkOnScreen() const override;

    double amountOfCheese = 3;
    bool playerJumping = false;
};

} // namespace papaGame::models::effects
