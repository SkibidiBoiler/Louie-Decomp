#pragma once
// AS: papaGame.models.enemies.Enemy34 — Burgerzilla: a stack of four ingredients that jumps around and loses one
// per hit.
#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy34 : public Enemy {
public:
    explicit Enemy34(obf_9G::obf_4R *gameObj) : Enemy(gameObj) {}

    void init(double xtile, double ytile, double id, double whichScreen, double whichTileset, double screenObjID,
              const std::vector<as3::Value> &extraParams) override;
    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    void removeEnemy() override;
    void killMe(double dir = 1) override;
    void stunEnemy() override;
    bool getHit(double damage = 1, double dir = 1, bool unused = false, const std::string &weapon = "none") override;
    void reactToHurtingPlayer() override;
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void updateObject() override;
    bool checkOnScreen() override;
    void jumpChar(bool collide = true) override;

    bool hasJumped = false;
    bool wasRunning = false;
    bool isJumpUnstunning = false;
    bool isSquashing = false;
    bool isReadyJump = false;
    double delayBeforeJump = 8;
    double delayTimer = 0;
    double dizzyFXID = -1;
    bool shouldFacePlayer = false;
    double numIngredients = 4;
    double startingXtile = 0;
    double targetXtile = 0;
    bool canDropParts = true;
};

} // namespace papaGame::models::enemies
