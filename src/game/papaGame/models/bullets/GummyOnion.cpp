#include "game/papaGame/models/bullets/GummyOnion.h"

#include <SDL3/SDL.h>

#include <cmath>

#include "game/as3.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::bullets {

// AS: GummyOnion.GummyOnion
void GummyOnion::init(double x_, double y_, double dirx_, double diry_, const std::string &type_, double id_,
                      double statAttack_, bool fromEnemy_, const std::string &fromWeaponName_)
{
    Bullet::init(x_, y_, dirx_, diry_, type_, id_, statAttack_, fromEnemy_, fromWeaponName_);
    facingDirX = dirx_;
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
    deadZoneRect1.x += x;
    deadZoneRect1.y += y;
    deadZoneRect2.x += x;
    deadZoneRect2.y += y;
    if (facingDirX == -1) {
        deadZoneRect1.x -= deadZoneRect1.width;
        deadZoneRect2.x -= deadZoneRect2.width;
        deadZoneRect2.x -= deadZoneRect1.width * 2;
    }
    SDL_Log("Object at %s,%s. Dead Zones start at %s, %s / %s, %s", as3::String(x).c_str(), as3::String(y).c_str(),
            as3::String(deadZoneRect1.x).c_str(), as3::String(deadZoneRect1.y).c_str(),
            as3::String(deadZoneRect2.x).c_str(), as3::String(deadZoneRect2.y).c_str());
}

// AS: GummyOnion.defineVars
void GummyOnion::defineVars()
{
    sheetname = "bullet_GummyOnion";
    sheetIsBitmap = false;
    shotname = "GummyOnion";
    width = 9;
    height = 9;
    playerheight = 9;
    spritewidth = 30;
    spriteheight = 30;
    spriteCenterX = 15;
    spriteCenterY = 15;
    spriteTargetX = 15;
    spriteTargetY = 15;
    sheetWidth = 24;
    sheetHeight = 2;
    collRect = Rect{-12, -12, 24, 24};
    heightmultiplier = 1;
    jumpspeed = 0;
    jumpstart = 4;
    gravity = 2;
    normalgravity = 2;
    speed = 10;
    vspeed = 0;
    basespeed = 10;
    flipBlitOffset = 30;
    animationFrame = 0;
    bounceX = true;
    bounceY = true;
    bounceDecayX = 0.9;
    bounceDecayY = 0.95;
    onlyCheckSolidSlopes = true;
    hasDeadZone1 = true;
    hasDeadZone2 = true;
    deadZoneRect1 = Rect{0, -200, 100, 200};
    deadZoneRect2 = Rect{100, -200, 100, 150};
}

// AS: GummyOnion.grabAnimationCycle — still at first, then spinning one way or the other per bounce.
const AnimationCycle &GummyOnion::grabAnimationCycle()
{
    if (numBounces == 0) {
        return cycleFrames("start0CycleFrames");
    }
    return cycleFrames("bounce" + as3::String(std::fmod(numBounces, 2)) + "CycleFrames");
}

// AS: GummyOnion.fireBullet — QUIRK: `diry` is set to itself (the angle argument is not used here).
void GummyOnion::fireBullet(double px, double py, double pdirx, double /*angle*/)
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

// AS: GummyOnion.updateObject
void GummyOnion::updateObject()
{
    bool burst = false;
    const double oldBounces = numBounces;
    onlyCheckSolidSlopes = isInDeadZone() && numBounces == 0;
    jumpChar();
    onlyCheckSolidSlopes = isInDeadZone() && numBounces == 0;
    moveChar(dirx, 0, 0);
    if (numBounces != oldBounces) {
        if (numBounces == 1) {
            bounceDecayY = 1.3;
            speed = 8;
        } else if (numBounces == 2) {
            bounceDecayY = 0.8;
            speed = 7;
        } else {
            bounceDecayY = 0.9;
            speed = 6;
        }
        if (isReallyOnScreen()) {
            gameObj->obf_7C->playSound("gummy_bounce" + as3::String(std::ceil(as3::random() * 3)) + ".wav", false,
                                       getProximityVolume(), getProximityPan());
        }
    }
    if (numBounces >= 4) {
        burst = true;
    }
    updateSprite();
    bool hitEnemy = false, hitObject = false, hitPlayer = false, hitBullet = false;
    if (fromEnemy) {
        hitPlayer = checkHittingPlayers();
    } else {
        hitEnemy = checkHittingEnemies();
        hitObject = checkHittingObjects(true);
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

// AS: GummyOnion.flyChar — straight flight at speed / vspeed, stopped by walls, clouds and the level's edges.
// (Nothing calls it in PL2.)
bool GummyOnion::flyChar(double, double, double, bool, bool)
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
    if (checkIfOnCloud()) {
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

// AS: GummyOnion.fadeShot — the burst hits everything in its larger rectangle once more.
void GummyOnion::fadeShot()
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
    managers::EffectManager &fx = *gameObj->obf_V;
    fx.addEffect(x, y - 7, "DustEffect");
    fx.addEffect(x - 4, y, "DustEffect");
    fx.addEffect(x + 4, y, "DustEffect");
    gameObj->obf_4l->removeBullet(id);
}

// AS: GummyOnion.checkHittingEnemies — one enemy per flight (the burst may hit several); the kills are counted
// but not used.
bool GummyOnion::checkHittingEnemies(double, double)
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

// AS: GummyOnion.checkHittingObjects
bool GummyOnion::checkHittingObjects(bool blockedToo)
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

// AS: GummyOnion.checkHittingPlayers
bool GummyOnion::checkHittingPlayers()
{
    characters::CustomerChar &player = *gameObj->playerObj;
    const Rect r{x + collRect.x, y + collRect.y, collRect.width, collRect.height};
    if (player.checkSpriteCollision(r) && !isStunned && !isDead && player.canGetHit()) {
        player.hurtPlayer(x < player.x ? 1 : -1, statAttack);
        return true;
    }
    return false;
}

// AS: GummyOnion.checkIfOnCloud — clouds do not stop it while it only checks solid slopes.
bool GummyOnion::checkIfOnCloud()
{
    const managers::MapManager &map = *gameObj->obf_4i;
    const double left = map.getTileProperty(leftX, downY, "thrublock");
    const double right = map.getTileProperty(rightX, downY, "thrublock");
    if ((left == 1 || right == 1) && ytile != downY && (!isClimbingLadder || !checkIfLadderBelow())) {
        return !onlyCheckSolidSlopes;
    }
    return false;
}

} // namespace papaGame::models::bullets
