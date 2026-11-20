#pragma once
// AS: papaGame.models.enemies.Enemy19 — the Blue Shroom: stands facing the player and ducks into the ground and back
// up on a timer (or at random); stomping it only bounces the player, and while it is down it cannot be hit.
#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy19 : public Enemy {
public:
    using Enemy::Enemy;

    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    void stunEnemy() override;
    void reactToHurtingPlayer() override {} // AS: Enemy19.reactToHurtingPlayer (empty)
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void updateObject() override;
    bool getHit(double damage = 1, double dir = 1, bool unused = false, const std::string &weapon = "none") override;

    bool isDropping = false;
    bool isRising = false;
    bool isDown = false;
    Rect downRect{-15, -10, 30, 20};
    Rect upRect{-15, -30, 30, 40};
    double upTimeMax = 30;
    double downTimeMax = 30;
    double stateTimer = 0;
};

} // namespace papaGame::models::enemies
