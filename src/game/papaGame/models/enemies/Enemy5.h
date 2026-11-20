#pragma once
// AS: papaGame.models.enemies.Enemy5 — the Kaiser onion: walks back and forth, turning at walls; it cannot be
// stunned, and hops back after hurting the player.
#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy5 : public Enemy {
public:
    using Enemy::Enemy;

    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    void reactToHurtingPlayer() override;
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void removeEnemy() override;
    void updateObject() override;

    bool hasJumped = false;
    bool wasRunning = false;
};

} // namespace papaGame::models::enemies
