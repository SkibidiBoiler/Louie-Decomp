#pragma once
// AS: papaGame.models.bullets.GummyOnion — Sarge Fan's gummy onion: it bounces higher, then lower, four times, slowing down, then bursts into dust.
#include "game/papaGame/models/Bullet.h"

namespace papaGame::models::bullets {

class GummyOnion : public Bullet {
public:
    using Bullet::Bullet;
    void init(double x, double y, double dirx, double diry, const std::string &type, double id, double statAttack,
              bool fromEnemy, const std::string &fromWeaponName) override;
    void defineVars() override;
    double grabFlipOffset() const override { return facingDirX == 1 ? flipBlitOffset : 0; } // AS: GummyOnion.grabFlipOffset
    const AnimationCycle &grabAnimationCycle() override;
    void fireBullet(double x, double y, double dirx, double angle);
    void updateObject() override;
    bool flyChar(double unused1, double unused2, double unused3, bool unused4 = false, bool unused5 = false);
    void fadeShot() override;
    bool checkHittingEnemies(double unused1 = 0, double unused2 = 0) override;
    bool checkHittingObjects(bool blockedToo = false) override;
    bool checkHittingPlayers() override;
    bool checkIfOnCloud() override;

    double useAngle = 0;
    std::string throwType;
    Rect blastRect{-15, -15, 30, 30};
    double hitEnemyID = -1;
};

} // namespace papaGame::models::bullets
