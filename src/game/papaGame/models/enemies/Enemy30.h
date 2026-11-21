#pragma once
// AS: papaGame.models.enemies.Enemy30 — the Pararadish: a radish floating down on a parachute (usually let out by a
// spawner); once down it hops after the player.
#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy30 : public Enemy {
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
    bool checkOnScreen() override;

    bool hasJumped = false;
    bool wasRunning = false;
    bool isJumpUnstunning = false;
    bool isSquashing = false;
    double delayBeforeJump = 4;
    double delayTimer = 0;
    bool isParachuting = true;
    bool isEndFlying = false;
    double dizzyFXID = -1;
};

} // namespace papaGame::models::enemies
