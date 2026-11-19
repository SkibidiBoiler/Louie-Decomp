#pragma once
// AS: papaGame.models.enemies.Enemy17 — the dill worm: hangs from the ceiling until the player passes beneath, drops,
// then crawls after the player.
#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy17 : public Enemy {
public:
    using Enemy::Enemy;

    void init(double xtile, double ytile, double id, double whichScreen, double whichTileset, double screenObjID,
              const std::vector<as3::Value> &extraParams) override;
    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    void stunEnemy() override;
    void reactToHurtingPlayer() override;
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void removeEnemy() override;
    void updateObject() override;
    void findStartupSurface();
    void killMe(double dir = 1) override;

    bool hasJumped = false;
    bool wasRunning = false;
    bool isJumpUnstunning = false;
    bool isHanging = false;
    bool isNoticing = false;
    bool isDropping = false;
    bool isSquashing = false;
    double dizzyFXID = -1;
};

} // namespace papaGame::models::enemies
