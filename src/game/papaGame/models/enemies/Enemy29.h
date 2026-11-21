#pragma once
// AS: papaGame.models.enemies.Enemy29 — the Laddish: a radish that climbs up and down a ladder, turning at its
// ends; stomped, it falls off, and when it recovers it climbs again.
#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy29 : public Enemy {
public:
    using Enemy::Enemy;

    void init(double xtile, double ytile, double id, double whichScreen, double whichTileset, double screenObjID,
              const std::vector<as3::Value> &extraParams) override;
    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    void stunEnemy() override;
    void reactToHurtingPlayer() override {} // AS: Enemy29.reactToHurtingPlayer (empty)
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void updateObject() override;
    bool checkOnScreen() override;

    bool hasJumped = false;
    bool wasRunning = false;
    bool isJumpUnstunning = false;
    double dizzyFXID = -1;
};

} // namespace papaGame::models::enemies
