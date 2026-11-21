#include "game/papaGame/models/bullets/BeakerBlob.h"

#include <cmath>

#include "game/as3.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::bullets {

// AS: BeakerBlob.BeakerBlob — the middle blob (dirx 0) faces right and jumps 2 higher.
void BeakerBlob::init(double x_, double y_, double dirx_, double diry_, const std::string &type_, double id_,
                      double statAttack_, bool fromEnemy_, const std::string &fromWeaponName_)
{
    Bullet::init(x_, y_, dirx_, diry_, type_, id_, statAttack_, fromEnemy_, fromWeaponName_);
    if (dirx_ != 0) {
        facingDirX = dirx_;
    } else {
        facingDirX = 1;
        jumpstart -= 2;
    }
    facingDirY = 0;
    useAngle = diry_;
    fireBullet(x_, y_, dirx_, useAngle);
    isWalking = true;
    jump = true;
    if (speed != 0 && vspeed == 0) {
        throwType = "forward";
    } else if (vspeed < 0 && speed != 0) {
        throwType = "45up";
    } else if (vspeed > 0 && speed != 0) {
        throwType = "45down";
    } else if (vspeed < 0 && speed == 0) {
        throwType = "up";
    } else if (vspeed > 0 && speed == 0) {
        throwType = "down";
    }
}

// AS: BeakerBlob.defineVars
void BeakerBlob::defineVars()
{
    sheetname = "bullet_BeakerBlob";
    sheetIsBitmap = false;
    shotname = "BeakerBlob";
    width = 6;
    height = 6;
    playerheight = 6;
    spritewidth = 16;
    spriteheight = 16;
    spriteCenterX = 8;
    spriteCenterY = 8;
    spriteTargetX = 8;
    spriteTargetY = 8;
    sheetWidth = 6;
    sheetHeight = 1;
    collRect = Rect{-8, -8, 16, 16};
    heightmultiplier = 1;
    jumpspeed = 0;
    jumpstart = -14;
    gravity = 2;
    normalgravity = 2;
    speed = 4;
    vspeed = 0;
    basespeed = 4;
    flipBlitOffset = 0;
    animationFrame = 0;
    bounceX = true;
    bounceY = true;
    bounceDecayX = 0.8;
    bounceDecayY = 0.8;
}

// AS: BeakerBlob.grabAnimationCycle
const AnimationCycle &BeakerBlob::grabAnimationCycle()
{
    return cycleFrames("animCycleFrames");
}

// AS: BeakerBlob.fireBullet — QUIRK: `diry` is set to itself (the angle argument is not used here).
void BeakerBlob::fireBullet(double px, double py, double pdirx, double /*angle*/)
{
    const display::GameDisplay &display = *gameObj->obf_71;
    x = px;
    y = py;
    dirx = pdirx;
    xtile = std::floor(px / display.tileWidth);
    ytile = std::floor(py / display.tileWidth);
    speed = basespeed;
    jumpspeed = jumpstart;
    isWalking = true;
    jump = true;
}

// AS: BeakerBlob.updateObject
void BeakerBlob::updateObject()
{
    bool burst = false;
    jumpChar();
    moveChar(dirx, 0, 0);
    if (!jump || onSlope != 0 || !isWalking) {
        burst = true; // it bursts where it lands
    }
    updateSprite();
    bool hitEnemy = false, hitObject = false, hitPlayer = false, hitBullet = false;
    if (fromEnemy) {
        hitPlayer = checkHittingPlayers();
    } else {
        hitEnemy = checkHittingEnemies();
        hitObject = checkHittingObjects(true);
        hitBullet = checkHittingBullets();
    }
    const display::GameDisplay &display = *gameObj->obf_71;
    if (!hitEnemy && !hitObject && !hitPlayer && !hitBullet) {
        if (x + spritewidth + 80 < display.currentXcoord) {
            fadeShot();
        } else if (x > display.currentXcoord + display.screenPxWidth + 80) {
            fadeShot();
        } else if (y > display.levelPxHeight) {
            fadeShot();
        } else if (burst) {
            fadeShot();
        }
    } else {
        fadeShot();
    }
}

// AS: BeakerBlob.flyChar — straight flight at speed / vspeed, stopped by walls, clouds and the level's edges.
// (Nothing calls it in PL2.)
bool BeakerBlob::flyChar(double, double, double, bool, bool)
{
    const display::GameDisplay &display = *gameObj->obf_71;
    const double tile = display.tileWidth;
    bool stopped = false;
    y += vspeed;
    x += speed;
    getMyCorners(x, y);
    if (speed > 0 && (HitUpRight || HitMidRight || HitDownRight)) {
        stopped = true;
    } else if (speed < 0 && (HitUpLeft || HitMidLeft || HitDownLeft)) {
        stopped = true;
    }
    if (vspeed > 0 && (HitDownLeft || HitDownRight)) {
        stopped = true;
    } else if (vspeed < 0 && (HitUpLeft || HitUpRight)) {
        stopped = true;
    }
    if (checkIfOnCloud() && ytile >= gameObj->playerObj->ytile - 1) {
        stopped = true;
    }
    const double maxX = display.levelTileWidth * tile;
    const double maxY = display.levelTileHeight * tile;
    if (x < 0) {
        x = 0;
        stopped = true;
    }
    if (x > maxX) {
        x = maxX;
        stopped = true;
    }
    if (y < 0) {
        y = 0;
        stopped = true;
    }
    if (y > maxY) {
        y = maxY;
        stopped = true;
    }
    xtile = std::floor(x / tile);
    ytile = std::floor(y / tile);
    return stopped;
}

// AS: BeakerBlob.fadeShot — the burst hits everything in its larger rectangle once more.
void BeakerBlob::fadeShot()
{
    isFading = true;
    collRect = blastRect;
    if (fromEnemy) {
        checkHittingPlayers();
    } else {
        checkHittingEnemies();
        checkHittingObjects();
        checkHittingBullets();
    }
    gameObj->obf_V->addEffect(x, y, "DustEffect");
    gameObj->obf_4l->removeBullet(id);
}

// AS: BeakerBlob.checkHittingEnemies — one enemy per flight (the burst may hit several); the kills are counted
// but not used.
bool BeakerBlob::checkHittingEnemies(double, double)
{
    bool hit = false;
    for (Enemy *e : gameObj->obf_E->enemies) {
        if (!e) {
            continue;
        }
        const Rect r{x + collRect.x, y + collRect.y, collRect.width, collRect.height};
        if (e->checkSpriteCollision(r) && !alreadyHitEnemy(e->id)) {
            if (e->id != hitEnemyID && (e->isDead || (!e->isHit && !e->resistBullets))) {
                enemyIDsHit.push_back(e->id);
                const bool landed = e->getHit(statAttack, dirx, true, fromWeaponName);
                addImpactEffect(landed);
                hit = true;
                hitEnemyID = e->id;
                if (!isFading) {
                    break;
                }
            }
        }
    }
    return hit;
}

// AS: BeakerBlob.checkHittingObjects
bool BeakerBlob::checkHittingObjects(bool blockedToo)
{
    bool hit = false;
    const Rect r{x + collRect.x, y + collRect.y, collRect.width, collRect.height};
    for (GameObject *o : gameObj->obf_7u->objects) {
        if (!o) {
            continue;
        }
        if (o->checkSpriteCollision(r) && o->acceptsDamage("bullet", fromWeaponName)) {
            o->getHit(statAttack);
            hit = true;
            if (!isFading) {
                break;
            }
        } else if (blockedToo && o->checkSpriteCollision(r) && o->blocksBullets) {
            hit = true;
            if (!isFading) {
                break;
            }
        }
    }
    return hit;
}

// AS: BeakerBlob.checkHittingPlayers
bool BeakerBlob::checkHittingPlayers()
{
    characters::CustomerChar &player = *gameObj->playerObj;
    const Rect r{x + collRect.x, y + collRect.y, collRect.width, collRect.height};
    if (player.checkSpriteCollision(r) && !isStunned && !isDead && player.canGetHit()) {
        player.hurtPlayer(x < player.x ? 1 : -1, statAttack);
        return true;
    }
    return false;
}


} // namespace papaGame::models::bullets
