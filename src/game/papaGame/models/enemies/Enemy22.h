#pragma once
// AS: papaGame.models.enemies.Enemy22 — the Bacobar: a tall bacon strip that walks between two columns (or
// stands) and is a wall: hits only make it wobble and a player inside it is pushed out and hurt. Stomped on its top,
// it crunches down to half height and runs faster; crunched, it can be beaten.
#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy22 : public Enemy {
public:
    using Enemy::Enemy;

    void init(double xtile, double ytile, double id, double whichScreen, double whichTileset, double screenObjID,
              const std::vector<as3::Value> &extraParams) override;
    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    bool getHit(double damage = 1, double dir = 1, bool unused = false, const std::string &weapon = "none") override;
    void saveEnemyState();
    void stunEnemy() override;
    void reactToHurtingPlayer() override;
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void removeEnemy() override;
    void updateObject() override;

    double startingXtile = 0;
    double targetXtile = 0;
    bool isCrunching = false;
    bool isCrunched = false;
    double crunchSpeed = 4;
    bool isStatic = false;
};

} // namespace papaGame::models::enemies
