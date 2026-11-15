#pragma once
// AS: papaGame.models.effects.PlayerCheckpointEffect — the stars that circle the player at a checkpoint, then fly off.
#include "game/papaGame/models/Effect.h"

namespace papaGame::models::effects {

class PlayerCheckpointEffect : public Effect {
public:
    using Effect::Effect;

    void init(double x, double y, double id, const std::string &subtype, bool followPlayer, double playerXoffset,
              double playerYoffset) override;
    void defineVars() override;
    const AnimationCycle &grabAnimationCycle() override;
    void updateObject() override;
    void stopSpinning();

    double radius = 48;
    double rotationDirection = -1;
    double rotationTimer = 0;
    double blimpAngle = 0;
    double flightCircleX = 0;
    double flightCircleY = 0;
    double xspeed = 0;
    double yspeed = 0;
    double spinTimer = 0;
    double spinTimerMax = 20;
};

} // namespace papaGame::models::effects
