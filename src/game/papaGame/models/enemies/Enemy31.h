#pragma once
// AS: papaGame.models.enemies.Enemy31 — the Cheese Wheel: rolls one way without stopping, bouncing down slopes;
// usually let out by a spawner. Any stomp beats it.
#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy31 : public Enemy {
public:
    using Enemy::Enemy;

    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    void stunEnemy() override;
    void reactToHurtingPlayer() override {} // AS: Enemy31.reactToHurtingPlayer (empty)
    const AnimationCycle &grabAnimationCycle() override;
    void removeEnemy() override;
    void updateObject() override;

    // The original's animCycleFrames array, whose one frame grabAnimationCycle turns with the roll.
    AnimationCycle animCycleFrames = cycleType2("anim", 1, 0, 0, {0});
    double totalFrames = 30;
};

} // namespace papaGame::models::enemies
