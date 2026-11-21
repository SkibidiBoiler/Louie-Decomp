#include "game/papaGame/models/objects/GameObject14.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::objects {

// AS: GameObject14.GameObject14 — each berry starts its sway at a random frame.
void GameObject14::init(double xtile_, double ytile_, double id_, double whichScreen_, double screenObjID_,
                        const std::vector<as3::Value> &extraParams)
{
    GameObject::init(xtile_, ytile_, id_, whichScreen_, screenObjID_, extraParams);
    startingYtile = ytile;
    diry = 1;
    cycleName = "hang";
    cycleFrame = std::floor(as3::random() * 12);
}

// AS: GameObject14.defineVars
void GameObject14::defineVars()
{
    sheetname = "object_dropberry";
    objectname = "Drop Berry";
    sheetIsBitmap = false;
    type = 14;
    spritewidth = 55;
    spriteheight = 55;
    spriteCenterX = 27;
    spriteCenterY = 27;
    spriteTargetX = 27;
    spriteTargetY = 27;
    sheetWidth = 23;
    sheetHeight = 1;
    collRect = {-20, -27, 40, 50};
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

// AS: GameObject14.endAnimationCycle
void GameObject14::endAnimationCycle()
{
    if (cycleName == "splat") {
        resetBerry();
    } else if (cycleName == "refill") {
        readyBerry();
    }
}

// AS: GameObject14.grabAnimationCycle
const AnimationCycle &GameObject14::grabAnimationCycle()
{
    if (isSplattering) {
        return splatCycleFrames;
    }
    if (jump) {
        return fallCycleFrames;
    }
    if (isResetting) {
        return refillCycleFrames;
    }
    if (playerDidStep) {
        return standCycleFrames;
    }
    return hangCycleFrames;
}

// AS: GameObject14.resetBerry — back at its place, growing again.
void GameObject14::resetBerry()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    jump = false;
    playerDidStep = false;
    isResetting = true;
    isSplattering = false;
    isSteppable = false;
    standTimer = 0;
    standingChars.clear();
    standingCharsLastX.clear();
    ytile = startingYtile;
    y = ytile * d.tileWidth + d.tileWidth / 2;
}

// AS: GameObject14.readyBerry
void GameObject14::readyBerry()
{
    jump = false;
    playerDidStep = false;
    isResetting = false;
    isSteppable = true;
}

// AS: GameObject14.splatBerry — its riders bounce off. QUIRK: stopStandingOnObject removes the rider from the
// list being walked, so with two riders the second is skipped (and then dropped from the list without being told).
void GameObject14::splatBerry()
{
    gameObj->obf_7C->playSound("watersplash.wav", false, getProximityVolume() / 3, getProximityPan());
    isSteppable = false;
    jump = false;
    isSplattering = true;
    for (size_t i = 0; i < standingChars.size(); ++i) {
        Character *c = standingChars[i];
        c->jump = true;
        c->jumpspeed = -12;
        stopStandingOnObject(*c);
    }
    standingChars.clear();
    standingCharsLastX.clear();
}

// AS: GameObject14.dropBerry — riders on a slope, a cloud or solid ground stay behind. QUIRK: as in splatBerry, the
// rider after one that is let go is not looked at. (Its debug trace() calls are left out.)
void GameObject14::dropBerry()
{
    jump = true;
    jumpspeed = 0;
    gameObj->obf_7C->playSound("dropberry_drop" + as3::String(std::ceil(as3::random() * 2)) + ".wav", false,
                               getProximityVolume(), getProximityPan());
    for (size_t i = 0; i < standingChars.size(); ++i) {
        Character *c = standingChars[i];
        if (c->onSlope || c->checkIfOnCloud() || c->HitDownLeft || c->HitDownRight) {
            stopStandingOnObject(*c);
        }
    }
}

// AS: GameObject14.checkOnScreen — four columns either side of the screen.
bool GameObject14::checkOnScreen()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 4 && xtile < d.currentXtileRight + 4;
}

// AS: GameObject14.stepOnObject
void GameObject14::stepOnObject(Character &c)
{
    GameObject::stepOnObject(c);
    if (c.whichMovingTile == this && c.isPlayer()) {
        if (!hasPlayer) {
            gameObj->obf_7C->playSound("dropberry_step" + as3::String(std::ceil(as3::random() * 3)) + ".wav", false,
                                       getProximityVolume(), getProximityPan());
        }
        hasPlayer = true;
        playerDidStep = true;
    }
}

// AS: GameObject14.stopStandingOnObject
void GameObject14::stopStandingOnObject(Character &c)
{
    GameObject::stopStandingOnObject(c);
    if (c.whichMovingTile != this && c.isPlayer()) {
        hasPlayer = false;
    }
}

// AS: GameObject14.updateObject — stood on for standTimerMax frames it drops; falling, it splats on landing (or
// below the level). Riders on a slope get off; while it falls they ride it, else they snap to its top.
void GameObject14::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    lastx = x;
    lasty = y;
    lastOnSlope = onSlope;
    if (isHit) {
        return;
    }
    if (playerDidStep && !jump) {
        ++standTimer;
        if (standTimer == standTimerMax) {
            dropBerry();
        }
    } else if (jump) {
        jumpChar();
        if (!jump || ytile >= d.levelTileHeight) {
            splatBerry();
        }
    }
    updateSprite();
    standOffset = spriteFrameOffsets[static_cast<size_t>(animationFrame)];
    for (int i = static_cast<int>(standingChars.size()) - 1; i >= 0; --i) {
        Character *c = standingChars[static_cast<size_t>(i)];
        if (c->onSlope) {
            stopStandingOnObject(*c);
        } else if (jump && jumpspeed > 0) {
            rideObject(*c, standOffset);
        } else {
            c->y = y - spriteTargetY - c->height + standOffset;
            c->ytile = std::floor(c->y / d.tileWidth);
        }
    }
}

// AS: GameObject14.rideObject — the rider moves down with it unless that would put it into the ground; then it hops
// off. (Its debug trace() calls are left out.)
void GameObject14::rideObject(Character &c, double offset)
{
    const double targetY = y - spriteTargetY - c.height + offset;
    c.getMyCorners(c.x, targetY);
    if ((!c.HitDownLeft || (c.SlopeDownRight > 0 && c.SlopeLeft > 0)) &&
        (!c.HitDownRight || (c.SlopeDownLeft > 0 && c.SlopeRight > 0))) {
        c.y = targetY;
        c.ytile = std::floor(c.y / gameObj->obf_71->tileWidth);
    } else {
        c.jump = true;
        c.jumpspeed = -6;
        stopStandingOnObject(c);
    }
}

} // namespace papaGame::models::objects
