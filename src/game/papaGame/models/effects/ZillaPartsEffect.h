#pragma once
// AS: papaGame.models.effects.ZillaPartsEffect — an ingredient knocked off Burgerzilla.
#include "game/papaGame/models/Effect.h"

namespace papaGame::models::effects {

class ZillaPartsEffect : public Effect {
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
