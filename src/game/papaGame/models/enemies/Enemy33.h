#pragma once
// AS: papaGame.models.enemies.Enemy33 — the Awesome Saucer: a flying saucer that glides between two columns on a
// sine wave, slowing into each turn, and drops sauce (SauceBullet) on a timer.
#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy33 : public Enemy {
public:
    using Enemy::Enemy;

    void init(double xtile, double ytile, double id, double whichScreen, double whichTileset, double screenObjID,
              const std::vector<as3::Value> &extraParams) override;
    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    void stunEnemy() override;
    void reactToHurtingPlayer() override {} // AS: Enemy33.reactToHurtingPlayer (empty)
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void updateObject() override;
    void shootSauce();

    bool hasJumped = false;
    bool wasRunning = false;
    bool isJumpUnstunning = false;
    double startingXtile = 0;
    double targetXtile = 0;
    double delayBeforeShooting = 90;
    double shootTimer = 0;
    bool wasFlying = false;
    double initialY = 0;
    double waveTimer = 0;
    double wavePeriod = 8;
    double waveAmplitude = 10;
};

} // namespace papaGame::models::enemies
