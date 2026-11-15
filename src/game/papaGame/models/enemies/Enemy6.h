#pragma once
// AS: papaGame.models.enemies.Enemy6 — the tomato: hops in place, alternating two jumps, squashing on landing.
#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy6 : public Enemy {
public:
    using Enemy::Enemy;

    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    void stunEnemy() override;
    void reactToHurtingPlayer() override;
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void removeEnemy() override;
    void updateObject() override;
    void killMe(double dir = 1) override;

    bool hasJumped = false;
    bool wasRunning = false;
    bool isJumpUnstunning = false;
    double whichJump = 1;
    bool didFirstJump = false;
    bool isSquashing = false;
    double initialDelay = 0;
    double landDelay = 0;
    double delayTimer = 0;
    double dizzyFXID = -1;
};

} // namespace papaGame::models::enemies
