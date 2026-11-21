#pragma once
// AS: papaGame.models.effects.RadishDirtEffect — a clod of dirt thrown up by a radish popping out
// of the ground; it vanishes when it lands.
#include "game/papaGame/models/Effect.h"

namespace papaGame::models::effects {

class RadishDirtEffect : public Effect {
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
