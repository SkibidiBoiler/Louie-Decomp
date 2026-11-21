#pragma once
// AS: papaGame.models.enemies.Enemy36 — the first Radley's Hideout boss (sheet "easyboss"): once the fight starts
// he runs back and forth, throws a Beaker every 120 frames and blocks every hit unless stunned; a stunned one can
// be beaten, which ends the boss music with the fanfare.
#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy36 : public Enemy {
public:
    using Enemy::Enemy;

    void defineVars() override;
    void setupParams(const std::vector<as3::Value> & /*params*/) override {} // AS: Enemy36.setupParams (no params)
    void killMe(double dir = 1) override;
    void stunEnemy() override;
    void reactToHurtingPlayer() override {} // AS: Enemy36.reactToHurtingPlayer (empty)
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void removeEnemy() override { Enemy::removeEnemy(); } // AS: Enemy36.removeEnemy (only the base's)
    bool getHit(double damage = 1, double dir = 1, bool unused = false, const std::string &weapon = "none") override;
    void updateObject() override;
    void sendTriggered(double state, bool pan, bool fromGlobal = false) override;

    bool isBlocking = false;
    bool isTriggered = false;
    double attackTimer = 0;
    double attackTimerMax = 120;
    double dizzyFXID = -1;
};

} // namespace papaGame::models::enemies
