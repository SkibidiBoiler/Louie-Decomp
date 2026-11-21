#pragma once
// AS: papaGame.models.enemies.Enemy18 — the Dill Weed: a pickle plant rooted in place that chomps forward, left and
// right in turn; its hit box grows with each chomp. Stomping only stuns it.
#include <string>

#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy18 : public Enemy {
public:
    using Enemy::Enemy;

    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    void stunEnemy() override;
    void reactToHurtingPlayer() override {} // AS: Enemy18.reactToHurtingPlayer (empty)
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void updateObject() override;
    void killMe(double dir = 1) override;

    bool hasJumped = false;
    bool wasRunning = false;
    bool isJumpUnstunning = false;
    bool isChomping = false;
    bool isUnchomping = false;
    std::string chompDirection = "Forward";
    Rect chompRectForward{-30, 0, 60, 40};
    Rect chompRectLeft{-40, 0, 60, 40};
    Rect chompRectRight{-20, 0, 60, 40};
    Rect normalCollRect{-15, -15, 30, 20};
};

} // namespace papaGame::models::enemies
