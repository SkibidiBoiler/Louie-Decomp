#pragma once
// AS: papaGame.models.enemies.Enemy15 — the leafy lark: flies back and forth between two columns, bobbing; a hit
// plucks it out of the air, and on the ground it hops towards where it faces.
#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy15 : public Enemy {
public:
    using Enemy::Enemy;

    void init(double xtile, double ytile, double id, double whichScreen, double whichTileset, double screenObjID,
              const std::vector<as3::Value> &extraParams) override;
    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    void stunEnemy() override;
    bool getHit(double damage = 1, double dir = 1, bool unused = false, const std::string &weapon = "none") override;
    void reactToHurtingPlayer() override;
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void removeEnemy() override;
    void updateObject() override;

    bool hasJumped = false;
    bool wasRunning = false;
    bool isJumpUnstunning = false;
    double dizzyFXID = -1;
    double startingXtile = 0;
    double targetXtile = 0;
    double delayBeforeJump = 30;
    double delayTimer = 0;
    bool wasFlying = false;
    bool isSquashing = false;
    double initialY = 0;
    double waveTimer = 0;
    double wavePeriod = 8;
    double waveAmplitude = 10;
    double lastFlap = 2;
};

} // namespace papaGame::models::enemies
