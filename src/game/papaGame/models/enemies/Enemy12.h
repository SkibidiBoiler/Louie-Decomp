#pragma once
// AS: papaGame.models.enemies.Enemy12 — the slider: winds up, slides off and slows to a stop, then turns round and
// slides back. Stomping it hurts it instead of stunning it.
#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy12 : public Enemy {
public:
    using Enemy::Enemy;

    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    void stunEnemy() override;
    void reactToHurtingPlayer() override {} // AS: Enemy12.reactToHurtingPlayer (empty)
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void removeEnemy() override;
    void startSliding();
    void updateObject() override;
    void killMe(double dir = 1) override;

    bool hasJumped = false;
    bool wasRunning = false;
    bool isWindingUp = false;
    double delayBeforeSlide = 18;
    double delayTimer = 0;
    double startingSlideSpeed = 8;
    double slideDecay = 3;
};

} // namespace papaGame::models::enemies
