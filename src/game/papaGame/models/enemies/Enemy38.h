#pragma once
// AS: papaGame.models.enemies.Enemy38 — Radley Madish's BBQ rocket: an enemy that homes in on the player, turning at
// most maxAngleDifference a frame and splashing where it hits a wall; a hit knocks it away spinning. Enemy39 (the
// mustard rocket) is the same with canHitBack: hit, it turns round and homes in on the boss instead.
#include <memory>

#include "game/papaGame/models/Enemy.h"

namespace engine { class SoundChannel; }

namespace papaGame::models::enemies {

class Enemy38 : public Enemy {
public:
    using Enemy::Enemy;

    void init(double xtile, double ytile, double id, double whichScreen, double whichTileset, double screenObjID,
              const std::vector<as3::Value> &extraParams) override;
    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    void stunEnemy() override;
    void reactToHurtingPlayer() override;
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override {} // AS: Enemy38.endAnimationCycle (empty)
    void updateObject() override;
    void fadeShot();
    void destroy() override;
    void setAnimationAngle();
    double radsToDegrees(double rads) const;
    double degreesToRads(double degrees) const;
    double matchNegativeSpace(double angle, double reference) const;
    void placeEnemy(double px, double py) override;
    void resetBulletSpeeds();
    bool getHit(double damage = 1, double dir = 1, bool unused = false, const std::string &weapon = "none") override;
    bool checkHittingEnemies();
    bool flyChar(double unused1, double unused2, double unused3, bool unused4 = false, bool unused5 = false);
    void jumpChar(bool collide = true) override;
    void killMe(double dir = 1) override;

    double useAngle = 0;
    // The original's animCycleFrames (type 1), whose loop / first / last frame setAnimationAngle sets to the frame
    // of the current angle.
    AnimationCycle animCycleFrames = cycleType1("anim", 1, 0, 0, 0, 0);
    double currentAngle = 2147483647;
    double smokeTimer = 0;
    double smokeTimerInterval = 2;
    double whichSmokeType = 1;
    Rect blastRect{-40, -40, 80, 80};
    bool gotOutOfCollision = false;
    double maxAngleDifference = 0.06;
    std::shared_ptr<engine::SoundChannel> rocketSound;
    bool isSeekingPlayer = true;
    bool isSeekingBoss = false;
    bool isRealigning = false;
    bool isDeflected = false;
    bool canHitBack = false;
    double bossID = -1;
    Rect hurtRect{-10, -10, 20, 20};
    double vspeed = 7;
    double basespeed = 7;
    std::string rocketType = "RocketBBQ";
};

} // namespace papaGame::models::enemies
