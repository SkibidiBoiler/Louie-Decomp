#pragma once
// AS: papaGame.models.effects.ChiliBitEffect — a bit of chili flying off a burst can.
#include "game/papaGame/models/Effect.h"

namespace papaGame::models::effects {

class ChiliBitEffect : public Effect {
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
