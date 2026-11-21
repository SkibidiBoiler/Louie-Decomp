#pragma once
// AS: papaGame.models.enemies.Enemy20 — the Thorn Shroom: like the Blue Shroom it faces the player and ducks into the
// ground and back up on a timer (or at random), but its thorns hurt a stomping player (it cannot be stunned).
#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy20 : public Enemy {
public:
    using Enemy::Enemy;

    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    void reactToHurtingPlayer() override {} // AS: Enemy20.reactToHurtingPlayer (empty)
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void updateObject() override;
    bool getHit(double damage = 1, double dir = 1, bool unused = false, const std::string &weapon = "none") override;

    bool isDropping = false;
    bool isRising = false;
    bool isDown = false;
    Rect downRect{-25, -10, 50, 20};
    Rect upRect{-25, -40, 50, 50};
    double upTimeMax = 30;
    double downTimeMax = 30;
    double stateTimer = 0;
};

} // namespace papaGame::models::enemies
