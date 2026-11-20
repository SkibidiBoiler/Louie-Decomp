#include "game/papaGame/models/objects/GameObject8.h"

#include <algorithm>
#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::objects {

// AS: GameObject8.GameObject8 — it heads for the target row.
void GameObject8::init(double xtile_, double ytile_, double id_, double whichScreen_, double screenObjID_,
                       const std::vector<as3::Value> &extraParams)
{
    GameObject::init(xtile_, ytile_, id_, whichScreen_, screenObjID_, extraParams);
    startingYtile = ytile;
    diry = startingYtile > targetYtile ? -1 : 1;
}

// AS: GameObject8.defineVars
void GameObject8::defineVars()
{
    sheetname = "object_blimp_vertical";
    objectname = "Vertical Blimp";
    sheetIsBitmap = false;
    type = 8;
    spritewidth = 96;
    spriteheight = 110;
    spriteCenterX = 48;
    spriteCenterY = 55;
    spriteTargetX = 48;
    spriteTargetY = 32;
    sheetWidth = 14;
    sheetHeight = 1;
    collRect = {-32, -16, 64, 80};
    width = 20;
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
    walkspeed = 3;
}

// AS: GameObject8.setupParams — [wait for the player, target row].
void GameObject8::setupParams(const std::vector<as3::Value> &params)
{
    if (params.empty()) {
        return;
    }
    waitForPlayer = params[0].asNumber() == 1;
    targetYtile = params.size() > 1 ? params[1].asNumber() : as3::NaN;
}

// AS: GameObject8.endAnimationCycle
void GameObject8::endAnimationCycle()
{
    if (cycleName == "startstand") {
        playerIsRiding = true;
    } else if (cycleName == "stopstand") {
        playerIsRiding = false;
    }
}

// AS: GameObject8.grabAnimationCycle — it sinks when stood on and rises when left.
const AnimationCycle &GameObject8::grabAnimationCycle()
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

// AS: GameObject8.checkOnScreen — four columns either side of the screen.
bool GameObject8::checkOnScreen()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 4 && xtile < d.currentXtileRight + 4;
}

// AS: GameObject8.stepOnObject
void GameObject8::stepOnObject(Character &c)
{
    GameObject::stepOnObject(c);
    if (c.whichMovingTile == this && c.isPlayer()) {
        hasPlayer = true;
    }
}

// AS: GameObject8.stopStandingOnObject
void GameObject8::stopStandingOnObject(Character &c)
{
    GameObject::stopStandingOnObject(c);
    if (c.whichMovingTile != this && c.isPlayer()) {
        hasPlayer = false;
    }
}

// AS: GameObject8.updateObject — once moving it speeds up to walkspeed and turns at both ends (a waiter stops
// back at its start when empty), carrying its riders up and down; idle, it bobs.
void GameObject8::updateObject()
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
