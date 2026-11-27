#pragma once
// AS: papaGame.models.effects.RomanTrailEffect — a puff of the roman candle's smoke trail.
#include "game/papaGame/models/Effect.h"

namespace papaGame::models::effects {

class RomanTrailEffect : public Effect {
public:
    using Effect::Effect;

    void defineVars() override;
    // AS: RomanTrailEffect.grabAnimationCycle
    const AnimationCycle &grabAnimationCycle() override { return cycleFrames("animCycleFrames"); }
    void endAnimationCycle() override;
    void updateObject() override { updateSprite(); } // AS: RomanTrailEffect.updateObject
};

} // namespace papaGame::models::effects
