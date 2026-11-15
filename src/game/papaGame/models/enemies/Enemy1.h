#pragma once
// AS: papaGame.models.enemies.Enemy1 — the onion: walks, turns at walls, can be stunned by stomping.
#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy1 : public Enemy {
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

    bool hasJumped = false;
    bool wasRunning = false;
    bool isJumpUnstunning = false;
    double dizzyFXID = -1;
};

} // namespace papaGame::models::enemies
