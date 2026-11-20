#pragma once
// AS: papaGame.models.bullets.SargePepper — Sarge's thrown pepper: lobbed in an arc (flat when the player is near, high when far), it
// explodes where it lands.
#include "game/papaGame/models/Bullet.h"

namespace papaGame::models::bullets {

class SargePepper : public Bullet {
public:
    using Bullet::Bullet;
    void init(double x, double y, double dirx, double diry, const std::string &type, double id, double statAttack,
              bool fromEnemy, const std::string &fromWeaponName) override;
    void defineVars() override;
    double grabFlipOffset() const override { return 0; } // AS: SargePepper.grabFlipOffset
    const AnimationCycle &grabAnimationCycle() override;
    void fireBullet(double x, double y, double dirx, double angle);
    void updateObject() override;
    bool flyChar(double unused1, double unused2, double unused3, bool unused4 = false, bool unused5 = false);
    void fadeShot() override;
    bool checkHittingPlayers() override;

    double useAngle = 0;
    std::string throwType;
    Rect blastRect{-25, -25, 50, 50};
    double hitEnemyID = -1;
};

} // namespace papaGame::models::bullets
