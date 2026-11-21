#include "game/papaGame/models/objects/GameObject15.h"

#include <algorithm>
#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::objects {

// AS: GameObject15.defineVars
void GameObject15::defineVars()
{
    sheetname = "object_onberry";
    objectname = "On Berry";
    sheetIsBitmap = false;
    type = 15;
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
    activeCycleFrames = cycleType2("active", 1, 0, 0, {3, 4, 5, 6, 7, 8, 9, 10, 9, 8, 7, 6, 5, 4});
    inactiveCycleFrames = cycleType1("inactive", 1, 0, 0, 0, 0);
    activatingCycleFrames = cycleType2("activating", 1, 0, -1, {0, 1, 2});
    deactivatingCycleFrames = cycleType2("deactivating", 1, 0, -1, {2, 1});
    isActivated = true;
    currentHealth = 3;
    maxHealth = 3;
    foreground = false;
    isSteppable = true;
    isSteppableByObjects = true;
    broadcastStyle = 1;
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
    walkspeed = 0;
}

// AS: GameObject15.setupParams — [starts up, upDuration, downDuration, mainTimer].
void GameObject15::setupParams(const std::vector<as3::Value> &params)
{
    if (params.size() > 0) {
        if (params[0].asNumber() == 1) {
            isActivated = true;
            isDeactivated = false;
            startedAsUp = 1;
        } else {
            isActivated = false;
            isDeactivated = true;
            startedAsUp = 0;
        }
    }
    if (params.size() > 1) {
        upDuration = params[1].asNumber();
    }
    if (params.size() > 2) {
        downDuration = params[2].asNumber();
    }
    if (params.size() > 3) {
        mainTimer = params[3].asNumber();
        startedMainTimer = params[3].asNumber();
    }
}

// AS: GameObject15.saveObjectData — it keeps its starting state, not its current one.
void GameObject15::saveObjectData()
{
    std::vector<as3::Value> row{as3::Value::num(type), as3::Value::num(xtile), as3::Value::num(ytile),
                                as3::Value::num(currentTriggers)};
    row.push_back(as3::Value::num(startedAsUp));
    row.push_back(as3::Value::num(upDuration));
    row.push_back(as3::Value::num(downDuration));
    row.push_back(as3::Value::num(startedMainTimer));
    gameObj->obf_AQ->currentScreenData->saveObjectState(id - 1, row);
}

// AS: GameObject15.destroy
void GameObject15::destroy()
{
    saveObjectData();
}

// AS: GameObject15.grabAnimationCycle
const AnimationCycle &GameObject15::grabAnimationCycle()
{
    if (isActivating) {
        return activatingCycleFrames;
    }
    if (isDeactivating) {
        return deactivatingCycleFrames;
    }
    if (isActivated) {
        if (mainTimer >= upDuration - 20) {
            return almostCycleFrames;
        }
        return activeCycleFrames;
    }
    return inactiveCycleFrames;
}

// AS: GameObject15.checkOnScreen — four columns either side of the screen.
bool GameObject15::checkOnScreen()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 4 && xtile < d.currentXtileRight + 4;
}

// AS: GameObject15.updateObject — the timer flips it up and down; its riders follow the sprite's top.
void GameObject15::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    lastx = x;
    lasty = y;
    lastOnSlope = onSlope;
    if (!isHit) {
        ++mainTimer;
        if (isActivated && mainTimer >= upDuration && !isActivating && !isDeactivating) {
            deactivateObject();
        } else if (isDeactivated && mainTimer >= downDuration && !isActivating && !isDeactivating) {
            activateObject();
        }
        updateSprite();
        standOffset = spriteFrameOffsets[static_cast<size_t>(animationFrame)];
        for (int i = static_cast<int>(standingChars.size()) - 1; i >= 0; --i) {
            Character *c = standingChars[static_cast<size_t>(i)];
            c->y = y - spriteTargetY - c->height + standOffset;
            c->ytile = std::floor(c->y / d.tileWidth);
        }
    }
}

// AS: GameObject15.finishDeactivatingObject
void GameObject15::finishDeactivatingObject()
{
    GameObject::finishDeactivatingObject();
    mainTimer = 0;
}

// AS: GameObject15.finishActivatingObject
void GameObject15::finishActivatingObject()
{
    GameObject::finishActivatingObject();
    if (isReallyOnScreen()) {
        if (type == 15) {
            gameObj->obf_7C->playSound("blue_berry.wav", false, getProximityVolume(), getProximityPan());
        } else {
            gameObj->obf_7C->playSound("orange_berry.wav", false, getProximityVolume(), getProximityPan());
        }
    }
    mainTimer = 0;
}

// AS: GameObject15.checkReallyOnScreen
bool GameObject15::checkReallyOnScreen() const
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile >= d.currentXtileLeft - 1 && xtile <= d.currentXtileRight + 1 && ytile >= d.currentYtileUp - 1 &&
           ytile <= d.currentYtileDown + 1;
}

// AS: GameObject15.checkStandingOnObject — the base test without the slope check; nothing stands on it while it is
// down.
bool GameObject15::checkStandingOnObject(Character &c)
{
    bool standing = true;
    const double charRight = c.x + c.width;
    const double charLeft = c.x - c.width;
    const double right = x + collRect.x + collRect.width;
    const double left = x + collRect.x;
    if (charRight < left || charLeft > right) {
        standing = false;
    } else if (c.jump && c.jumpspeed < 0) {
        standing = false;
    } else if (isGrabbed) {
        standing = false;
    }
    if (isDeactivated) {
        standing = false;
    }
    if (!standing) {
        stopStandingOnObject(c);
    }
    return standing;
}

// AS: GameObject15.stepOnObject — only while it is up.
void GameObject15::stepOnObject(Character &c)
{
    if (!isDeactivated && !isActivating) {
        if (std::find(standingChars.begin(), standingChars.end(), &c) == standingChars.end()) {
            standingChars.push_back(&c);
            standingCharsLastX.push_back(c.x);
        }
        c.jump = false;
        c.onMovingTile = true;
        c.whichMovingTile = this;
    }
}

} // namespace papaGame::models::objects
