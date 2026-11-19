#pragma once
// AS: papaGame.models.effects.SmokeRingEffect — the smoke out of a launcher's muzzle; it can follow the player.
#include "game/papaGame/models/Effect.h"

namespace papaGame::models::effects {

class SmokeRingEffect : public Effect {
public:
    using Effect::Effect;

    void defineVars() override;
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void updateObject() override;
};

} // namespace papaGame::models::effects
