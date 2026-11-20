#pragma once
// AS: papaGame.models.bullets.GiantPepper — Sarge's giant pepper: a bigger lobbed pepper with a wider blast.
#include "game/papaGame/models/Bullet.h"

namespace papaGame::models::bullets {

class GiantPepper : public Bullet {
public:
    using Bullet::Bullet;
    void init(double x, double y, double dirx, double diry, const std::string &type, double id, double statAttack,
              bool fromEnemy, const std::string &fromWeaponName) override;
    void defineVars() override;
    double grabFlipOffset() const override { return 0; } // AS: GiantPepper.grabFlipOffset
    const AnimationCycle &grabAnimationCycle() override;
    void fireBullet(double x, double y, double dirx, double angle);
    void updateObject() override;
    bool flyChar(double unused1, double unused2, double unused3, bool unused4 = false, bool unused5 = false);
    void fadeShot() override;
    bool checkHittingPlayers() override;

    double useAngle = 0;
    std::string throwType;
    Rect blastRect{-60, -60, 120, 120};
    double hitEnemyID = -1;
};

} // namespace papaGame::models::bullets
