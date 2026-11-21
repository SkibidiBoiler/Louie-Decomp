#include "game/papaGame/models/objects/GameObject13.h"

#include <algorithm>
#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::objects {

// AS: GameObject13.GameObject13
void GameObject13::init(double xtile_, double ytile_, double id_, double whichScreen_, double screenObjID_,
                        const std::vector<as3::Value> &extraParams)
{
    GameObject::init(xtile_, ytile_, id_, whichScreen_, screenObjID_, extraParams);
    startingYtile = ytile;
    diry = 1;
}

// AS: GameObject13.defineVars
void GameObject13::defineVars()
{
    sheetname = "object_blimp_drop_small";
    objectname = "Dropping Blimp Small";
    sheetIsBitmap = false;
    type = 13;
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
    walkspeed = 3;
}

// AS: GameObject13.setupParams — [target row].
void GameObject13::setupParams(const std::vector<as3::Value> &params)
{
    if (params.empty()) {
        return;
    }
    targetYtile = params[0].asNumber();
}

// AS: GameObject13.endAnimationCycle
void GameObject13::endAnimationCycle()
{
    if (cycleName == "startstand") {
        playerIsRiding = true;
    } else if (cycleName == "stopstand") {
        playerIsRiding = false;
    }
}

// AS: GameObject13.grabAnimationCycle — it sinks when stood on and rises when left.
const AnimationCycle &GameObject13::grabAnimationCycle()
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

// AS: GameObject13.checkOnScreen — four columns either side of the screen.
bool GameObject13::checkOnScreen()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 4 && xtile < d.currentXtileRight + 4;
}

// AS: GameObject13.stepOnObject
void GameObject13::stepOnObject(Character &c)
{
    GameObject::stepOnObject(c);
    if (c.whichMovingTile == this && c.isPlayer()) {
        hasPlayer = true;
    }
}

// AS: GameObject13.stopStandingOnObject
void GameObject13::stopStandingOnObject(Character &c)
{
    GameObject::stopStandingOnObject(c);
    if (c.whichMovingTile != this && c.isPlayer()) {
        hasPlayer = false;
    }
}

// AS: GameObject13.updateObject — stood on at its start, it sinks to its target row while the player rides it
// and rises back (then stops) once the player leaves; idle, it bobs.
void GameObject13::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    lastx = x;
    lasty = y;
    lastOnSlope = onSlope;
    if (isHit) {
        return;
    }
    if (hasPlayer && !isMoving && ytile == startingYtile) {
        isMoving = true;
    }
    if (isMoving) {
        const double oldDir = diry;
        diry = hasPlayer ? 1 : -1;
        if (diry != oldDir) {
            speed = 0;
        }
        if (speed < walkspeed && std::fmod(gameObj->obf_8o->gameplayTimer, 3) == 0) {
            ++speed;
        }
        moveChar(0, diry, 0, true);
        const double bottomY = std::max(startingYtile, targetYtile) * d.tileWidth + d.tileWidth / 2;
        const double topY = std::min(startingYtile, targetYtile) * d.tileWidth + d.tileWidth / 2;
        if (diry == 1 && y > bottomY) {
            y = bottomY;
            ytile = std::floor(y / d.tileWidth);
            speed = 0;
        } else if (diry == -1 && y < topY) {
            y = topY;
            ytile = std::floor(y / d.tileWidth);
            speed = 0;
            if (!hasPlayer) {
                isMoving = false;
            }
        }
    }
    if (!isMoving) {
        ++waveTimer;
        y = initialY + std::sin(waveTimer / wavePeriod) * waveAmplitude;
        ytile = std::floor(y / d.tileWidth);
    } else {
        waveTimer = 0;
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
