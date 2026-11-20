#include "game/papaGame/models/bullets/GiantPepper.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::bullets {

// AS: GiantPepper.GiantPepper
void GiantPepper::init(double x_, double y_, double dirx_, double diry_, const std::string &type_, double id_,
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
}

// AS: GiantPepper.defineVars
void GiantPepper::defineVars()
{
    sheetname = "bullet_GiantPepper";
    sheetIsBitmap = false;
    shotname = "GiantPepper";
    width = 6;
    height = 6;
    playerheight = 6;
    spritewidth = 75;
    spriteheight = 75;
    spriteCenterX = 38;
    spriteCenterY = 38;
    spriteTargetX = 38;
    spriteTargetY = 38;
    sheetWidth = 1;
    sheetHeight = 2;
    collRect = Rect{-20, -20, 40, 40};
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
}

// AS: GiantPepper.grabAnimationCycle
const AnimationCycle &GiantPepper::grabAnimationCycle()
{
    return cycleFrames(facingDirX == 1 ? "rightCycleFrames" : "leftCycleFrames");
}

// AS: GiantPepper.fireBullet — QUIRK: `diry` is set to itself (the angle argument is not used here).
void GiantPepper::fireBullet(double px, double py, double pdirx, double /*angle*/)
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

// AS: GiantPepper.updateObject — lobbed in an arc, it bursts where it lands, on the player or off screen. (The original
// tests `checkOnScreen(this) || true`.)
void GiantPepper::updateObject()
{
    jumpChar();
    moveChar(dirx, 0, 0);
    const bool landed = !jump || onSlope != 0 || !isWalking;
    updateSprite();
    bool hitPlayer = false;
    if (fromEnemy) {
        hitPlayer = checkHittingPlayers();
    }
    const display::GameDisplay &display = *gameObj->obf_71;
    if (!hitPlayer) {
        if (x + spritewidth + 80 < display.currentXcoord) {
            fadeShot();
        } else if (x > display.currentXcoord + display.screenPxWidth + 80) {
            fadeShot();
        } else if (y > display.levelPxHeight) {
            fadeShot();
        } else if (landed) {
            fadeShot();
        }
    } else {
        fadeShot();
    }
}

// AS: GiantPepper.flyChar — straight flight at speed / vspeed, stopped by walls, clouds and the level's edges.
// (Nothing calls it in PL2.)
bool GiantPepper::flyChar(double, double, double, bool, bool)
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

// AS: GiantPepper.fadeShot — the burst hits the player in its larger rectangle once more.
void GiantPepper::fadeShot()
{
    isFading = true;
    collRect = blastRect;
    if (fromEnemy) {
        checkHittingPlayers();
    }
    gameObj->obf_V->addEffect(x, y, "ExplosionEffect");
    gameObj->obf_7C->playSound("bombexplosion.wav");
    gameObj->obf_4l->removeBullet(id);
}

// AS: GiantPepper.checkHittingPlayers
bool GiantPepper::checkHittingPlayers()
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
