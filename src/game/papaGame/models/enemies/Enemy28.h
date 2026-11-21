#pragma once
// AS: papaGame.models.enemies.Enemy28 — the Radish: walks back and forth after the player; a planted one hides in
// the ground (no hit box) until the player comes near, then pops out, throwing up dirt.
#include <climits>

#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy28 : public Enemy {
public:
    using Enemy::Enemy;

    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    void stunEnemy() override;
    void reactToHurtingPlayer() override;
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void updateObject() override;

    bool hasJumped = false;
    bool wasRunning = false;
    bool isJumpUnstunning = false;
    double dizzyFXID = -1;
    bool isPlanted = false;
    bool isPoppingOut = false;
    Rect regularRect{-15, -20, 30, 40};
    Rect plantedRect{static_cast<double>(INT_MIN), 0, 1, 1}; // far away: nothing touches it
};

} // namespace papaGame::models::enemies
