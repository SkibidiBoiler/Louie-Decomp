#include "game/papaGame/models/objects/GameObject7.h"

#include <algorithm>
#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::objects {

// AS: GameObject7.GameObject7 — it heads for the target column. (Its debug trace() calls are left out.)
void GameObject7::init(double xtile_, double ytile_, double id_, double whichScreen_, double screenObjID_,
                       const std::vector<as3::Value> &extraParams)
{
    GameObject::init(xtile_, ytile_, id_, whichScreen_, screenObjID_, extraParams);
    startingXtile = xtile;
    dirx = startingXtile > targetXtile ? -1 : 1;
}

// AS: GameObject7.defineVars
void GameObject7::defineVars()
{
    sheetname = "object_blimp_horizontal_small";
    objectname = "Horizonal Blimp Small";
    sheetIsBitmap = false;
    type = 7;
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

// AS: GameObject7.setupParams — [wait for the player, target column].
void GameObject7::setupParams(const std::vector<as3::Value> &params)
{
    if (params.empty()) {
        return;
    }
    waitForPlayer = params[0].asNumber() == 1;
    targetXtile = params.size() > 1 ? params[1].asNumber() : as3::NaN;
}

// AS: GameObject7.endAnimationCycle
void GameObject7::endAnimationCycle()
{
    if (cycleName == "startstand") {
        playerIsRiding = true;
    } else if (cycleName == "stopstand") {
        playerIsRiding = false;
    }
}

// AS: GameObject7.grabAnimationCycle — it sinks when stood on and rises when left.
const AnimationCycle &GameObject7::grabAnimationCycle()
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

// AS: GameObject7.checkOnScreen — four columns either side of the screen.
bool GameObject7::checkOnScreen()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 4 && xtile < d.currentXtileRight + 4;
}

// AS: GameObject7.stepOnObject
void GameObject7::stepOnObject(Character &c)
{
    GameObject::stepOnObject(c);
    if (c.whichMovingTile == this && c.isPlayer()) {
        hasPlayer = true;
    }
}

// AS: GameObject7.stopStandingOnObject
void GameObject7::stopStandingOnObject(Character &c)
{
    GameObject::stopStandingOnObject(c);
    if (c.whichMovingTile != this && c.isPlayer()) {
        hasPlayer = false;
    }
}

// AS: GameObject7.updateObject — once moving it speeds up to walkspeed, turns at both ends (a waiter stops back at
// its start when empty) and drags its riders along; idle, it bobs.
void GameObject7::updateObject()
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
        const double oldX = x;
        const double oldDir = dirx;
        moveChar(dirx, 0, 0, true);
        const double rightEndX = std::max(startingXtile, targetXtile) * d.tileWidth + d.tileWidth / 2;
        const double leftEndX = std::min(startingXtile, targetXtile) * d.tileWidth + d.tileWidth / 2;
        if (dirx == 1 && x > rightEndX) {
            x = rightEndX;
            xtile = std::floor(x / d.tileWidth);
            dirx = -1;
            speed = 0;
            if (xtile == startingXtile && !hasPlayer && waitForPlayer) {
                isMoving = false;
            }
        } else if (dirx == -1 && x < leftEndX) {
            x = leftEndX;
            xtile = std::floor(x / d.tileWidth);
            dirx = 1;
            speed = 0;
            if (xtile == startingXtile && !hasPlayer && waitForPlayer) {
                isMoving = false;
            }
        }
        const double newX = x;
        for (int i = static_cast<int>(standingChars.size()) - 1; i >= 0; --i) {
            Character *c = standingChars[static_cast<size_t>(i)];
            double carry = std::abs(newX - oldX);
            if (c->isWalking && c->walkingDir == dirx) {
                carry -= 4;
            }
            if (carry > 0) {
                c->moveChar(oldDir, 0, 0, false, false, true, carry);
            }
            standingCharsLastX[static_cast<size_t>(i)] = c->x;
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
