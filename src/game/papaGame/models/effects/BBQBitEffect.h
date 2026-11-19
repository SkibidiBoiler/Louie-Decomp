#pragma once
// AS: papaGame.models.effects.BBQBitEffect — a bit of BBQ sauce flying off a burst chili can.
#include "game/papaGame/models/Effect.h"

namespace papaGame::models::effects {

class BBQBitEffect : public Effect {
public:
    using Effect::Effect;

    void init(double x, double y, double id, const std::string &subtype, bool followPlayer, double playerXoffset,
              double playerYoffset) override;
    void defineVars() override;
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void updateObject() override;
};

} // namespace papaGame::models::effects
