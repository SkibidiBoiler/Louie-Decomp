#pragma once
// AS: papaGame.models.enemies.Enemy32 — the Jellyback: a harmless jelly that walks between two columns (or stands);
// stomping or hitting it only makes it bounce, throwing the player high. It cannot be killed.
#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy32 : public Enemy {
public:
    using Enemy::Enemy;

    void init(double xtile, double ytile, double id, double whichScreen, double whichTileset, double screenObjID,
              const std::vector<as3::Value> &extraParams) override;
    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    void killMe(double /*dir*/ = 1) override {} // AS: Enemy32.killMe (empty)
    bool getHit(double damage = 1, double dir = 1, bool unused = false, const std::string &weapon = "none") override;
    void stunEnemy() override;
    void reactToHurtingPlayer() override {} // AS: Enemy32.reactToHurtingPlayer (empty)
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void updateObject() override;

    bool hasJumped = false;
    bool wasRunning = false;
    bool isJumpUnstunning = false;
    double targetTileX = -1;
    double leftTileX = -1;
    double rightTileX = -1;
    bool didTrack = false;
};

} // namespace papaGame::models::enemies
