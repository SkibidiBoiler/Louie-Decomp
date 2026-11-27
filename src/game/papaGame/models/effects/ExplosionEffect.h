#pragma once
// AS: papaGame.models.effects.ExplosionEffect — the pepper bomb's explosion, played once.
#include "game/papaGame/models/Effect.h"

namespace papaGame::models::effects {

class ExplosionEffect : public Effect {
public:
    using Effect::Effect;

    void defineVars() override;
    // AS: ExplosionEffect.grabAnimationCycle
    const AnimationCycle &grabAnimationCycle() override { return cycleFrames("animCycleFrames"); }
    void endAnimationCycle() override;
    void updateObject() override { updateSprite(); } // AS: ExplosionEffect.updateObject
};

} // namespace papaGame::models::effects
