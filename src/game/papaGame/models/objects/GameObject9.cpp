#include "game/papaGame/models/objects/GameObject9.h"

#include <algorithm>
#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::objects {

// AS: GameObject9.GameObject9 — it heads for the target row.
void GameObject9::init(double xtile_, double ytile_, double id_, double whichScreen_, double screenObjID_,
                       const std::vector<as3::Value> &extraParams)
{
    GameObject::init(xtile_, ytile_, id_, whichScreen_, screenObjID_, extraParams);
    startingYtile = ytile;
    diry = startingYtile > targetYtile ? -1 : 1;
}

// AS: GameObject9.defineVars
void GameObject9::defineVars()
{
    sheetname = "object_blimp_vertical_small";
    objectname = "Vertical Blimp Small";
    sheetIsBitmap = false;
    type = 9;
    spritewidth = 64;
    spriteheight = 73;
    spriteCenterX = 32;
    spriteCenterY = 36;
    spriteTargetX = 32;
    spriteTargetY = 32;
    sheetWidth = 14;
    sheetHeight = 1;
    collRect = {-20, -16, 40, 50};
    width = 15;
    height = 15;
    playerheight = 15;
    isActivated = true;
    currentHealth = 3;
    maxHealth = 3;
    foreground = false;
    isSteppable = true;
    standingChars.clear();
    standingCharsLastX.clear();
    isPushable = false;
    isGrabbable = false;
    jumpspeed = 0;
    jumpstart = -16;
    jumpgravity = 2;
    normalgravity = 2;
    throwSpeed = 8;
    bounceY = false;
    statAttack = 500;
    canBeShot = false;
    canBeMeleed = false;
    canBeBrokenByObjects = false;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 0;
    walkspeed = 2;
}

// AS: GameObject9.setupParams — [wait for the player, target row].
void GameObject9::setupParams(const std::vector<as3::Value> &params)
{
    if (params.empty()) {
        return;
    }
    waitForPlayer = params[0].asNumber() == 1;
    targetYtile = params.size() > 1 ? params[1].asNumber() : as3::NaN;
}

// AS: GameObject9.endAnimationCycle
void GameObject9::endAnimationCycle()
{
    if (cycleName == "startstand") {
        playerIsRiding = true;
    } else if (cycleName == "stopstand") {
        playerIsRiding = false;
    }
}

// AS: GameObject9.grabAnimationCycle — it sinks when stood on and rises when left.
const AnimationCycle &GameObject9::grabAnimationCycle()
{
    if (!standingChars.empty() && !playerIsRiding) {
        return startstandCycleFrames;
    }
    if (standingChars.empty() && playerIsRiding) {
        return stopstandCycleFrames;
    }
    if (playerIsRiding) {
        return standCycleFrames;
    }
    return normalCycleFrames;
}

// AS: GameObject9.checkOnScreen — four columns either side of the screen.
bool GameObject9::checkOnScreen()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 4 && xtile < d.currentXtileRight + 4;
}

// AS: GameObject9.stepOnObject
void GameObject9::stepOnObject(Character &c)
{
    GameObject::stepOnObject(c);
    if (c.whichMovingTile == this && c.isPlayer()) {
        hasPlayer = true;
    }
}

// AS: GameObject9.stopStandingOnObject
void GameObject9::stopStandingOnObject(Character &c)
{
    GameObject::stopStandingOnObject(c);
    if (c.whichMovingTile != this && c.isPlayer()) {
        hasPlayer = false;
    }
}

// AS: GameObject9.updateObject — once moving it speeds up to walkspeed and turns at both ends (a waiter stops
// back at its start when empty), carrying its riders up and down; idle, it bobs.
void GameObject9::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    lastx = x;
    lasty = y;
    lastOnSlope = onSlope;
    if (isHit) {
        return;
    }
    if ((hasPlayer || !waitForPlayer) && !isMoving) {
        isMoving = true;
    }
    if (isMoving) {
        if (speed < walkspeed && std::fmod(gameObj->obf_8o->gameplayTimer, 2) == 0) {
            ++speed;
        }
        moveChar(0, diry, 0, true);
        const double bottomY = std::max(startingYtile, targetYtile) * d.tileWidth + d.tileWidth / 2;
        const double topY = std::min(startingYtile, targetYtile) * d.tileWidth + d.tileWidth / 2;
        if (diry == 1 && y > bottomY) {
            y = bottomY;
            ytile = std::floor(y / d.tileWidth);
            diry = -1;
            speed = 0;
            if (ytile == startingYtile && !hasPlayer && waitForPlayer) {
                isMoving = false;
            }
        } else if (diry == -1 && y < topY) {
            y = topY;
            ytile = std::floor(y / d.tileWidth);
            diry = 1;
            speed = 0;
            if (ytile == startingYtile && !hasPlayer && waitForPlayer) {
                isMoving = false;
            }
        }
    }
    if (!isMoving) {
        ++waveTimer;
        y = initialY + std::sin(waveTimer / wavePeriod) * waveAmplitude;
        ytile = std::floor(y / d.tileWidth);
    }
    updateSprite();
    standOffset = spriteFrameOffsets[static_cast<size_t>(animationFrame)];
    for (int i = static_cast<int>(standingChars.size()) - 1; i >= 0; --i) {
        Character *c = standingChars[static_cast<size_t>(i)];
        c->y = y - spriteTargetY - c->height + standOffset;
        c->ytile = std::floor(c->y / d.tileWidth);
    }
}

} // namespace papaGame::models::objects
