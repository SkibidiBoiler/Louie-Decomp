#pragma once
// AS: papaGame.models.enemies.Enemy25 — the Swiss Zack: a cheese crusher that slams down from the ceiling and rises
// again on a timer; it cannot be hurt or killed, and it counts as seen once it comes on screen.
#include <vector>

#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy25 : public Enemy {
public:
    using Enemy::Enemy;

    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    void killMe(double /*dir*/ = 1) override {} // AS: Enemy25.killMe (empty)
    void reactToHurtingPlayer() override {}     // AS: Enemy25.reactToHurtingPlayer (empty)
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void updateObject() override;
    // AS: Enemy25.getHit — never hurt.
    bool getHit(double = 1, double = 1, bool = false, const std::string & = "none") override { return false; }
    bool checkOnScreen() override;

    bool isDropping = false;
    bool isRising = false;
    bool isDown = false;
    Rect downRect{-30, 146, 60, 60}; // never used
    Rect upRect{-30, -6, 60, 60};
    // The top of the crusher's hit box for each animation frame.
    std::vector<double> collYoffsets{-6, -4, 1, 10, 24, 40, 60, 83, 111, 142, 137, 139, 140, 141, 138, 127, 109};
    double upTimeMax = 30;
    double downTimeMax = 30;
    double stateTimer = 0;
    bool didTrack = false;
};

} // namespace papaGame::models::enemies
