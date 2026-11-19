#pragma once
// AS: papaGame.models.Bullet (extends Character) — a projectile: the customers' thrown and shot weapons, the
// enemies' shots and the cheese cannon's. Hits enemies (or the player, when an enemy fired it) and objects.
#include <string>
#include <vector>

#include "game/papaGame/models/Character.h"

namespace engine { struct Sheet; }

namespace papaGame::models {

class Bullet : public Character {
public:
    // AS: Bullet(gameObj, x, y, dirx, diry, type, id, statAttack, fromEnemy, fromWeaponName); the subclasses'
    // constructors run their own part in init().
    using Character::Character;
    virtual void init(double x, double y, double dirx, double diry, const std::string &type, double id,
                      double statAttack, bool fromEnemy, const std::string &fromWeaponName);

    void setCritical();
    bool alreadyHitEnemy(double enemyID) const;
    virtual bool checkHittingEnemies(double unused1 = 0, double unused2 = 0);
    virtual bool checkHittingPlayers();
    virtual void addImpactEffect(bool hitSomething = false);
    virtual void fadeShot();
    virtual bool checkHittingObjects(bool blockedToo = false);
    virtual void checkIfCollecting() {} // AS: Bullet.checkIfCollecting (empty)
    void checkBounce();
    void updateObject() override;
    virtual double grabFlipOffset() const;
    bool isInDeadZone() const;
    virtual void getHit() {}                              // AS: Bullet.getHit (empty)
    virtual bool checkHittingBullets() { return false; } // AS: Bullet.checkHittingBullets
    const engine::Sheet *getSheet() const;                // port: BulletManager.bulletSheets[sheetIndex]

    // AS: `type`, the class name as a String ("Basketball"); Character.type is a Number in the port.
    std::string bulletType;
    double facingDirX = 0, facingDirY = 0;
    bool isThrown = false, throwBounce = false, throwBounce2 = false;
    double collectY = 0;
    double blitOffsetRight = 0, blitOffsetLeft = 60, blitOffset45Right = 120, blitOffset45Left = 180;
    double blitOffsetUp = 240, blitOffsetDown = 300, blitOffset45DownLeft = 300, blitOffset45DownRight = 300;
    std::string shotname;
    bool isFading = false, isCriticalHit = false;
    double vspeed = 0, basespeed = 0, maxspeedx = 0, maxspeedy = 0, accelx = 0, accely = 0, acceldivide = 0;
    bool foreground = true, isLocal = true;
    double statAttack = 1;
    bool giveStatusEffect = false;
    std::string giveStatusType;
    bool canPassThroughEnemies = false;
    std::vector<double> enemyIDsHit;
    bool fromEnemy = false;
    std::string fromWeaponName = "none";
    bool hasDeadZone1 = false, hasDeadZone2 = false; // port: deadZoneRect1 / 2 are not null
    Rect deadZoneRect1, deadZoneRect2;
    bool playerCanDeflect = false;
};

} // namespace papaGame::models
