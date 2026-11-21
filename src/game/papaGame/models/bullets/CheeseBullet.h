#pragma once
// AS: papaGame.models.bullets.CheeseBullet — the cheese cannon's shot: it flies straight and splats; a player
// it hits gets stuck in cheese.
#include "game/papaGame/models/Bullet.h"

namespace papaGame::models::bullets {

class CheeseBullet : public Bullet {
public:
    using Bullet::Bullet;
    void init(double x, double y, double dirx, double diry, const std::string &type, double id, double statAttack,
              bool fromEnemy, const std::string &fromWeaponName) override;
    void defineVars() override;
    double grabFlipOffset() const override { return 0; } // AS: CheeseBullet.grabFlipOffset
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
    Rect blastRect{-10, -6, 20, 12};
    double hitEnemyID = -1;
};

} // namespace papaGame::models::bullets
