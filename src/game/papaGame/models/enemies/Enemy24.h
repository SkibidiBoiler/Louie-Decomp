#pragma once
// AS: papaGame.models.enemies.Enemy24 — a Baco Bite: one of three bacon pieces a beaten Bacoburn breaks into; it
// hops back and forth, and any stomp beats it.
#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy24 : public Enemy {
public:
    using Enemy::Enemy;

    void init(double xtile, double ytile, double id, double whichScreen, double whichTileset, double screenObjID,
              const std::vector<as3::Value> &extraParams) override;
    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    void stunEnemy() override;
    void reactToHurtingPlayer() override {} // AS: Enemy24.reactToHurtingPlayer (empty)
    const AnimationCycle &grabAnimationCycle() override;
    void updateObject() override;

    bool hasJumped = false;
    bool wasRunning = false;
    double whichBite = 1;
    double delayBeforeJump = 15;
    double delayTimer = 0;
};

} // namespace papaGame::models::enemies
