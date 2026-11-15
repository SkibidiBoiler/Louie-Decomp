#pragma once
// AS: papaGame.models.enemies.Enemy2 — the brown onion: like Enemy1, but it can pace between its tile and a target tile.
#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy2 : public Enemy {
public:
    explicit Enemy2(obf_9G::obf_4R *gameObj) : Enemy(gameObj) {}
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

    bool hasJumped = false;
    bool wasRunning = false;
    bool isJumpUnstunning = false;
    double targetTileX = -1;
    double leftTileX = -1;
    double rightTileX = -1;
    double dizzyFXID = -1;
};

} // namespace papaGame::models::enemies
