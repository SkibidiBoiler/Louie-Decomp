#pragma once
// AS: papaGame.models.enemies.Enemy3 — the army onion: walks, turning at walls and edges. Stomped, it collapses
// into its helmet and runs on, faster, kicking up dust.
#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy3 : public Enemy {
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
    bool isStomped = false;
    bool isCollapsing = false;
    double stompedSpeed = 4;
};

} // namespace papaGame::models::enemies
