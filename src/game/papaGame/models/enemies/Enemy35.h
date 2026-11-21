#pragma once
// AS: papaGame.models.enemies.Enemy35 — Sarge, the boss of Sarge's Lair. Dropped into the arena by the "Start Fight"
// trigger, he jumps about and throws peppers (giant ones once hurt); each stomp makes him roll across the arena
// and then sit on blimps; at one hit left he also calls in Kaiser onions.
#include <vector>

#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy35 : public Enemy {
public:
    using Enemy::Enemy;

    void init(double xtile, double ytile, double id, double whichScreen, double whichTileset, double screenObjID,
              const std::vector<as3::Value> &extraParams) override;
    void defineVars() override;
    void setupParams(const std::vector<as3::Value> & /*params*/) override {} // AS: Enemy35.setupParams (empty)
    void stunEnemy() override;
    void reactToHurtingPlayer() override {} // AS: Enemy35.reactToHurtingPlayer (empty)
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void sendTriggered(double state, bool pan, bool fromGlobal = false) override;
    void startRolling();
    void stopRolling();
    void updateObject() override;
    void finishLanding();
    void startThrowing(bool giant = false, double style = -1);
    void finishThrowing(bool giant = false);
    void decideJump(double dir = 0);
    void addStaticBlimps();
    void removeBlimps();
    void addSecondBlimps();
    bool getHit(double damage = 1, double dir = 1, bool unused = false, const std::string &weapon = "none") override;
    void killMe(double dir = 1) override;
    void throwPepper();
    void lookForBlimps();
    void handleSpawningEnemies();

    bool hasJumped = false;
    bool wasRunning = false;
    bool isJumpUnstunning = false;
    bool isLanding = false;
    bool isRolling = false;
    bool isStartRolling = false;
    bool isThrowingSmall = false;
    bool isThrowingLarge = false;
    double throwStyle = 0;
    bool lastWasGiant = false;
    bool isBouncingAndRolling = false;
    bool isDropping = false;
    bool hasDropped = false;
    bool isMinorHurt = false;
    double rollTimer = 0;
    double rollTimerMax = 200;
    double rollSpeed = 14;
    double bounceRollSpeed = 9;
    std::vector<double> blimps;
    double throwsSinceJump = 0;
    bool lookingForBlimp = false;
    double spawnedEnemies = 0;
    double maxToSpawn = 6;
    double spawnTimer = 0;
    double spawnFrequency = 60;
    std::vector<double> spawns;
    double lastTimeThrownOrJumped = 0;
    // [object type, x tile, the blimp's second parameter]; y tile 0 when added.
    std::vector<std::vector<double>> staticBlimpCoords{{4, 14, 23}, {4, 20, 23}, {4, 27, 23}};
    std::vector<std::vector<double>> secondBlimpCoords{{4, 14, 23}, {4, 20, 23}, {4, 27, 23},
                                                       {5, 32, 21}, {4, 24, 18}, {5, 16, 18}};
};

} // namespace papaGame::models::enemies
