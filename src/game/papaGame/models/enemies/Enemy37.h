#pragma once
// AS: papaGame.models.enemies.Enemy37 — Radley Madish, the Radley's Hideout boss: he flies after the player on his
// propeller firing BBQ rockets (Enemy38) and now and then a mustard rocket (Enemy39); a mustard rocket hit back at
// him knocks him down. On the ground he runs about and can be stunned, then hit; after a while he takes off again.
// He has 3 hit points: with 3 he lets out a Pararadish at a time, with 2 up to three, with 1 a Burgerzilla and
// fires BBQ rockets in bursts of three.
#include <memory>

#include "game/papaGame/models/Enemy.h"

namespace engine { class SoundChannel; }

namespace papaGame::models::enemies {

class Enemy37 : public Enemy {
public:
    using Enemy::Enemy;

    void init(double xtile, double ytile, double id, double whichScreen, double whichTileset, double screenObjID,
              const std::vector<as3::Value> &extraParams) override;
    void defineVars() override;
    void setupParams(const std::vector<as3::Value> & /*params*/) override {} // AS: Enemy37.setupParams (no params)
    void stunEnemy() override;
    void reactToHurtingPlayer() override {} // AS: Enemy37.reactToHurtingPlayer (empty)
    void sendTriggered(double state, bool pan, bool fromGlobal = false) override;
    bool getHit(double damage = 1, double dir = 1, bool unused = false, const std::string &weapon = "none") override;
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void startGrabbingRocket();
    void loadRocket();
    void shootRocket();
    void updateObject() override;
    bool isIdle() const;
    void flyChar();
    void handleSpawningEnemies();
    void unregisterSpawn(double type, double id) override;
    void killMe(double dir = 1) override;
    void destroy() override;

    bool isTriggered = false;
    bool isShooting = false;
    bool isLoadingBBQ = false;
    bool isLoadingMustard = false;
    bool isGrabbingRocket = false;
    bool isReadyJump = false;
    bool isDropping = false;
    bool isLanding = false;
    bool isRising = false;
    bool isQuickLoadingBBQ = false;
    double quickLoadCounter = 0;
    double quickLoadCounterMax = 3;
    std::string useWhichRocket = "BBQ";
    double rocketCounter = 0;
    double grabRocketTimer = 0;
    double grabRocketTimerMax = 120;
    double initialY = 0;
    double waveTimer = 0;
    double wavePeriod = 8;
    double waveAmplitude = 10;
    double dizzyFXID = -1;
    double propellerFXID = -1;
    std::shared_ptr<engine::SoundChannel> copterSound;
    double flyTargetYoffset = 180;
    double flyTargetXoffset = 180;
    double riseTargetY = 0;
    double spawnedEnemies = 0;
    double maxToSpawn = 3;
    double spawnTimer = 0;
    double spawnFrequency = 40;
    std::vector<double> spawns;
    double spawnedBurgerzillas = 0;
    double maxBurgerzillasToSpawn = 1;
    double burgerSpawnTimer = 0;
    double burgerSpawnFrequency = 30;
    std::vector<double> burgerSpawns;
    double runTimer = 0;
    double maxRunTimer = 120;
    double starburstTimer = 0;
    double starburstTimerMax = 3;
};

} // namespace papaGame::models::enemies
