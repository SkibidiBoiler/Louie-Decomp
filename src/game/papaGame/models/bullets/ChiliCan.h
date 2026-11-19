#pragma once
// AS: papaGame.models.bullets.ChiliCan — Rico's chili can: lobbed in an arc, it bursts where it lands, splashing
// chili on the tiles.
#include "game/papaGame/models/Bullet.h"

namespace papaGame::models::bullets {

class ChiliCan : public Bullet {
public:
    using Bullet::Bullet;
    void init(double x, double y, double dirx, double diry, const std::string &type, double id, double statAttack,
              bool fromEnemy, const std::string &fromWeaponName) override;
    void defineVars() override;
    double grabFlipOffset() const override { return 0; } // AS: ChiliCan.grabFlipOffset
    const AnimationCycle &grabAnimationCycle() override;
    void fireBullet(double x, double y, double dirx, double angle);
    void updateObject() override;
    bool flyChar(double unused1, double unused2, double unused3, bool unused4 = false, bool unused5 = false);
    void fadeShot() override;
    bool checkHittingEnemies(double unused1 = 0, double unused2 = 0) override;
    bool checkHittingObjects(bool blockedToo = false) override;
    bool checkHittingPlayers() override;
    bool checkIfOnCloud() override;
    void addSplatter();

    double useAngle = 0;
    std::string throwType;
    Rect blastRect{-20, -20, 40, 40};
    double hitEnemyID = -1;
};

} // namespace papaGame::models::bullets
