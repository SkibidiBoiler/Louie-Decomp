#include "game/papaGame/models/objects/GameObject12.h"

#include <algorithm>
#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::objects {

// AS: GameObject12.GameObject12
void GameObject12::init(double xtile_, double ytile_, double id_, double whichScreen_, double screenObjID_,
                        const std::vector<as3::Value> &extraParams)
{
    GameObject::init(xtile_, ytile_, id_, whichScreen_, screenObjID_, extraParams);
    startingYtile = ytile;
    diry = 1;
}

// AS: GameObject12.defineVars
void GameObject12::defineVars()
{
    sheetname = "object_blimp_drop";
    objectname = "Dropping Blimp";
    sheetIsBitmap = false;
    type = 12;
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

// AS: GameObject12.setupParams — [target row].
void GameObject12::setupParams(const std::vector<as3::Value> &params)
{
    if (params.empty()) {
        return;
    }
    targetYtile = params[0].asNumber();
}

// AS: GameObject12.endAnimationCycle
void GameObject12::endAnimationCycle()
{
    if (cycleName == "startstand") {
        playerIsRiding = true;
    } else if (cycleName == "stopstand") {
        playerIsRiding = false;
    }
}

// AS: GameObject12.grabAnimationCycle — it sinks when stood on and rises when left.
const AnimationCycle &GameObject12::grabAnimationCycle()
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

// AS: GameObject12.checkOnScreen — four columns either side of the screen.
bool GameObject12::checkOnScreen()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 4 && xtile < d.currentXtileRight + 4;
}

// AS: GameObject12.stepOnObject
void GameObject12::stepOnObject(Character &c)
{
    GameObject::stepOnObject(c);
    if (c.whichMovingTile == this && c.isPlayer()) {
        hasPlayer = true;
    }
}

// AS: GameObject12.stopStandingOnObject
void GameObject12::stopStandingOnObject(Character &c)
{
    GameObject::stopStandingOnObject(c);
    if (c.whichMovingTile != this && c.isPlayer()) {
        hasPlayer = false;
    }
}

// AS: GameObject12.updateObject — stood on at its start, it sinks to its target row while the player rides it
// and rises back (then stops) once the player leaves; idle, it bobs.
void GameObject12::updateObject()
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
