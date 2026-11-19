#pragma once
// AS: papaGame.models.enemies.Enemy7 — the Roma tomato: hops forward without stopping, alternating two jumps,
// squashing on landing.
#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy7 : public Enemy {
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
    bool isSquashing = false;
    double dizzyFXID = -1;
};

} // namespace papaGame::models::enemies
