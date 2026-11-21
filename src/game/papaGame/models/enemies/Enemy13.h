#pragma once
// AS: papaGame.models.enemies.Enemy13 — the Brussel Lark: flies round and round a circle; any stomp kills it.
#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy13 : public Enemy {
public:
    using Enemy::Enemy;

    void init(double xtile, double ytile, double id, double whichScreen, double whichTileset, double screenObjID,
              const std::vector<as3::Value> &extraParams) override;
    void defineVars() override;
    bool checkOnScreen() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    void stunEnemy() override;
    void reactToHurtingPlayer() override {} // AS: Enemy13.reactToHurtingPlayer (empty)
    const AnimationCycle &grabAnimationCycle() override;
    void removeEnemy() override;
    void updateObject() override;
    void flyChar();

    bool hasJumped = false;
    bool wasRunning = false;
    double tileRadius = 2;
    double rotationDirection = 1;
    double rotationTimer = 0;
    double blimpAngle = 0;
    double flightCircleX = 0;
    double flightCircleY = 0;
    double lastFlap = 1;
};

} // namespace papaGame::models::enemies
