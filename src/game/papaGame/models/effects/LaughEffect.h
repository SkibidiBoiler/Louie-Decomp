#pragma once
// AS: papaGame.models.effects.LaughEffect — one letter of the boss's "BWAHAHAHA" (subtype "B", "W", "A", "H" or
// "Point"): it rises 330 px and fades.
#include "game/papaGame/models/Effect.h"

namespace papaGame::models::effects {

class LaughEffect : public Effect {
public:
    using Effect::Effect;

    void init(double x, double y, double id, const std::string &subtype, bool followPlayer, double playerXoffset,
              double playerYoffset) override;
    void defineVars() override;
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void updateObject() override;

    double initialY = 0;
    double ydist = 330;
    double targetY = 0;
    double timeToRise = 48;
    double animTimer = 0;
};

} // namespace papaGame::models::effects
