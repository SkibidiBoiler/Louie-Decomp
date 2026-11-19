#pragma once
// AS: papaGame.models.bullets.Banana — Akari's banana: it curves like the roman candle until it hits something, then
// bounces about and bursts into bits.
#include "game/papaGame/models/Bullet.h"

namespace papaGame::models::bullets {

class Banana : public Bullet {
public:
    using Bullet::Bullet;
    void init(double x, double y, double dirx, double diry, const std::string &type, double id, double statAttack,
              bool fromEnemy, const std::string &fromWeaponName) override;
    void defineVars() override;
    double grabFlipOffset() const override { return 0; } // AS: Banana.grabFlipOffset
    const AnimationCycle &grabAnimationCycle() override;
    void fireBullet(double x, double y, double dirx, double angle);
    void updateObject() override;
    bool flyChar(double unused1, double unused2, double unused3, bool unused4 = false, bool unused5 = false);
    void fadeShot() override;
    bool checkHittingEnemies(double unused1 = 0, double unused2 = 0) override;
    bool checkHittingObjects(bool blockedToo = false) override;
    bool checkHittingPlayers() override;

    double useAngle = 0;
    std::string throwType;
    Rect blastRect{-10, -10, 20, 20};
    double hitEnemyID = -1;
    double arcTimer = 0, arcTimerMax = 1;
    bool didHitSurface = false;
};

} // namespace papaGame::models::bullets
