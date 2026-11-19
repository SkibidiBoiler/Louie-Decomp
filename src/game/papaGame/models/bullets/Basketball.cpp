#include "game/papaGame/models/bullets/Basketball.h"

#include <SDL3/SDL.h>

#include "game/as3.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::bullets {

// AS: Basketball.Basketball
void Basketball::init(double x_, double y_, double dirx_, double diry_, const std::string &type_, double id_,
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

// AS: Basketball.defineVars
void Basketball::defineVars()
{
    sheetname = "bullet_Basketball";
    sheetIsBitmap = false;
    shotname = "Basketball";
    width = 9;
    height = 9;
    playerheight = 9;
    spritewidth = 26;
    spriteheight = 26;
    spriteCenterX = 13;
    spriteCenterY = 13;
    spriteTargetX = 13;
    spriteTargetY = 13;
    sheetWidth = 1;
    sheetHeight = 2;
    collRect = Rect{-11, -11, 22, 22};
    heightmultiplier = 1;
    jumpspeed = 0;
    jumpstart = -16;
    gravity = 2;
    normalgravity = 2;
    speed = 10;
    vspeed = 0;
    basespeed = 10;
    flipBlitOffset = 0;
    animationFrame = 0;
    bounceX = true;
    bounceY = true;
    bounceDecayX = 0.8;
    bounceDecayY = 0.8;
    onlyCheckSolidSlopes = true;
    hasDeadZone1 = true;
    hasDeadZone2 = true;
    deadZoneRect1 = Rect{0, -150, 100, 200};
    deadZoneRect2 = Rect{100, -150, 100, 150};
}

// AS: Basketball.grabAnimationCycle
const AnimationCycle &Basketball::grabAnimationCycle()
{
    return cycleFrames(facingDirX == 1 ? "rightCycleFrames" : "leftCycleFrames");
}

// AS: Basketball.fireBullet — QUIRK: `diry` is set to itself (the angle argument is not used here).
void Basketball::fireBullet(double px, double py, double pdirx, double /*angle*/)
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

// AS: Basketball.updateObject — it bounces along (a sound per bounce while on screen) and bursts after two
// bounces, on a hit, or off the level.
void Basketball::updateObject()
{
    bool burst = false;
    const double oldDirx = dirx;
    const double oldJumpspeed = jumpspeed;
    onlyCheckSolidSlopes = isInDeadZone() && numBounces == 0;
    jumpChar();
    onlyCheckSolidSlopes = isInDeadZone() && numBounces == 0;
    moveChar(dirx, 0, 0);
    if (numBounces >= 2) {
        burst = true;
    }
    if (jumpspeed < oldJumpspeed || dirx != oldDirx) {
        if (isReallyOnScreen()) {
            gameObj->obf_7C->playSound(as3::random() > 0.5 ? "basketball_bounce1.wav" : "basketball_bounce2.wav",
                                       false, getProximityVolume(), getProximityPan());
        }
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

// AS: Basketball.flyChar — straight flight at speed / vspeed, stopped by walls, clouds and the level's edges.
// (Nothing calls it in PL2.)
bool Basketball::flyChar(double, double, double, bool, bool)
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

// AS: Basketball.fadeShot — the burst hits everything in its larger rectangle once more.
void Basketball::fadeShot()
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

// AS: Basketball.checkHittingEnemies — one enemy per flight (the burst may hit several); the kills are counted
// but not used.
bool Basketball::checkHittingEnemies(double, double)
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

// AS: Basketball.checkHittingObjects
bool Basketball::checkHittingObjects(bool blockedToo)
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

// AS: Basketball.checkHittingPlayers
bool Basketball::checkHittingPlayers()
{
    characters::CustomerChar &player = *gameObj->playerObj;
    const Rect r{x + collRect.x, y + collRect.y, collRect.width, collRect.height};
    if (player.checkSpriteCollision(r) && !isStunned && !isDead && player.canGetHit()) {
        player.hurtPlayer(x < player.x ? 1 : -1, statAttack);
        return true;
    }
    return false;
}

// AS: Basketball.checkIfOnCloud — clouds do not stop it while it only checks solid slopes.
bool Basketball::checkIfOnCloud()
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
