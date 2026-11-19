#pragma once
// AS: papaGame.models.enemies.Enemy9 — the back of a party sub: the sub's parts (Enemy9, 10, 11) walk as a group,
// turning at walls and edges, and all stop while any of them is stunned.
#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy9 : public Enemy {
public:
    using Enemy::Enemy;

    void init(double xtile, double ytile, double id, double whichScreen, double whichTileset, double screenObjID,
              const std::vector<as3::Value> &extraParams) override;
    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    void receiveGroupMessage(const std::string &message, double value) override;
    void stunEnemy() override;
    void reactToHurtingPlayer() override {} // AS: Enemy9.reactToHurtingPlayer (empty)
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void removeEnemy() override;
    void updateObject() override;
    void adjustFrameSync(const std::string &newName, const std::string &oldName, double firstFrame,
                         const AnimationCycle &cycle) override;

    bool hasJumped = false;
    bool wasRunning = false;
    bool isJumpUnstunning = false;
    double dizzyFXID = -1;
    std::vector<double> stunnedIDs;
    std::vector<double> groupEnemyIDs;
    bool gotGroupIDs = false;
};

} // namespace papaGame::models::enemies
