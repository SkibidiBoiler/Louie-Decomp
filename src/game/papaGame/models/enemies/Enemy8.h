#pragma once
// AS: papaGame.models.enemies.Enemy8 — the runt tomato: hops back and forth, turning round before each jump. Any stun
// hurts it.
#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy8 : public Enemy {
public:
    using Enemy::Enemy;

    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    void stunEnemy() override;
    void reactToHurtingPlayer() override {} // AS: Enemy8.reactToHurtingPlayer (empty)
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void removeEnemy() override;
    void updateObject() override;
    void killMe(double dir = 1) override;

    bool hasJumped = false;
    bool wasRunning = false;
    bool isSquashing = false;
    double delayBeforeJump = 30;
    double delayTimer = 0;
};

} // namespace papaGame::models::enemies
