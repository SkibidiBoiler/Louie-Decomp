#pragma once
// AS: papaGame.models.enemies.Enemy21 — the Bow Shroom: like the Blue Shroom it ducks into the ground and back up,
// and while up it shoots arrows (a set number per rise, optionally only when the player is in sight).
#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy21 : public Enemy {
public:
    using Enemy::Enemy;

    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    void stunEnemy() override;
    void reactToHurtingPlayer() override {} // AS: Enemy21.reactToHurtingPlayer (empty)
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void fireArrow();
    void updateObject() override;
    bool getHit(double damage = 1, double dir = 1, bool unused = false, const std::string &weapon = "none") override;

    bool isDropping = false;
    bool isRising = false;
    bool isDown = false;
    Rect downRect{-15, -10, 30, 20};
    Rect upRect{-15, -35, 30, 45};
    double upTimeMax = 30;
    double downTimeMax = 30;
    double stateTimer = 0;
    bool shouldTurnToFace = false;
    double maxArrows = 1;
    double shotArrows = 0;
    bool hasArrow = true;
    double shootDelay = 0;
    bool onlyShootIfSeesPlayer = false;
    bool isPulling = false;
    bool isShooting = false;
};

} // namespace papaGame::models::enemies
