#pragma once
// AS: papaGame.models.enemies.Enemy23 — the Bacoburn: a tall bacon strip like the Bacobar that walks between two
// columns and is a wall (hits make it wobble, a player inside it is pushed out and hurt), but a stomp, slide or
// ground pound beats it at once and breaks it into three Baco Bites (Enemy24).
#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy23 : public Enemy {
public:
    using Enemy::Enemy;

    void init(double xtile, double ytile, double id, double whichScreen, double whichTileset, double screenObjID,
              const std::vector<as3::Value> &extraParams) override;
    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    bool getHit(double damage = 1, double dir = 1, bool unused = false, const std::string &weapon = "none") override;
    void stunEnemy() override;
    void reactToHurtingPlayer() override;
    void addBacoBites();
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void removeEnemy() override;
    void updateObject() override;

    double startingXtile = 0;
    double targetXtile = 0;
};

} // namespace papaGame::models::enemies
