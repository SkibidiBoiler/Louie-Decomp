#pragma once
// AS: papaGame.models.enemies.Enemy27 — the Pepper Jack: stands until the player comes near, then charges,
// speeding up; once past the player it skids to a stop and charges back. It jumps over gaps towards the player.
#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy27 : public Enemy {
public:
    using Enemy::Enemy;

    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    void stunEnemy() override;
    void reactToHurtingPlayer() override;
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void updateObject() override;
    void checkIfAtEdge(double speed, double dir) override;
    void killMe(double dir = 1) override;

    bool hasJumped = false;
    bool wasRunning = false;
    bool isJumpUnstunning = false;
    bool isChasing = false;
    bool isSkidding = false;
    bool isNoticing = false;
    double dizzyFXID = -1;
    double maxSpeed = 10;
    double skidTimer = 0;
    double skidDecay = 3;
};

} // namespace papaGame::models::enemies
