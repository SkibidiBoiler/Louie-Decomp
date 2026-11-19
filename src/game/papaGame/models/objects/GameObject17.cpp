#include "game/papaGame/models/objects/GameObject17.h"

#include <algorithm>
#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::objects {

// AS: GameObject17.defineVars
void GameObject17::defineVars()
{
    sheetname = "object_switch";
    objectname = "Switch";
    sheetIsBitmap = false;
    type = 17;
    spritewidth = 64;
    spriteheight = 32;
    spriteCenterX = 32;
    spriteCenterY = 16;
    spriteTargetX = 16;
    spriteTargetY = 16;
    sheetWidth = 10;
    sheetHeight = 1;
    collRect = {-16, -16, 64, 32};
    width = 15;
    height = 15;
    playerheight = 15;
    activeCycleFrames = cycleType2("active", 1, 0, 0, {0});
    inactiveCycleFrames = cycleType2("inactive", 1, 0, 0, {9});
    activatingCycleFrames = cycleType2("activating", 1, 0, -1, {8, 7, 6, 5, 4, 3, 2, 1});
    deactivatingCycleFrames = cycleType2("deactivating", 1, 0, -1, {1, 2, 3, 4, 5, 6, 7, 8});
    isActivated = true;
    currentHealth = 3;
    maxHealth = 3;
    foreground = false;
    isSteppable = true;
    isSteppableByObjects = true;
    broadcastStyle = 1;
    standingChars.clear();
    standingCharsLastX.clear();
    standingWalkSpeed = 8;
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

// AS: GameObject17.setupParams — [up (1) or pressed, broadcastStyle]
void GameObject17::setupParams(const std::vector<as3::Value> &params)
{
    if (params.size() > 0) {
        if (params[0].asNumber() == 1) {
            isActivated = true;
            isDeactivated = false;
        } else {
            isActivated = false;
            isDeactivated = true;
        }
    }
    if (params.size() > 1) {
        broadcastStyle = params[1].asNumber();
    }
}

// AS: GameObject17.saveObjectData
void GameObject17::saveObjectData()
{
    std::vector<as3::Value> row{as3::Value::num(type), as3::Value::num(xtile), as3::Value::num(ytile),
                                as3::Value::num(currentTriggers)};
    row.push_back(as3::Value::num(isActivated ? 1 : 0));
    row.push_back(as3::Value::num(broadcastStyle));
    gameObj->obf_AQ->currentScreenData->saveObjectState(id - 1, row);
}

// AS: GameObject17.destroy
void GameObject17::destroy()
{
    saveObjectData();
}

// AS: GameObject17.grabAnimationCycle
const AnimationCycle &GameObject17::grabAnimationCycle()
{
    if (isActivating) {
        return activatingCycleFrames;
    }
    if (isDeactivating) {
        return deactivatingCycleFrames;
    }
    if (isActivated) {
        return activeCycleFrames;
    }
    return inactiveCycleFrames;
}

// AS: GameObject17.checkOnScreen — four columns either side of the screen.
bool GameObject17::checkOnScreen()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 4 && xtile < d.currentXtileRight + 4;
}

// AS: GameObject17.updateObject — whoever stands on it sinks with the button.
void GameObject17::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    lastx = x;
    lasty = y;
    lastOnSlope = onSlope;
    if (!isHit) {
        updateSprite();
        standOffset = spriteFrameOffsets[static_cast<size_t>(animationFrame)];
        for (int i = static_cast<int>(standingChars.size()) - 1; i >= 0; --i) {
            Character *c = standingChars[static_cast<size_t>(i)];
            c->y = y - spriteTargetY - c->height + standOffset;
            c->ytile = std::floor(c->y / d.tileWidth);
        }
    }
}

// AS: GameObject17.deactivateObject — pressed, with a click.
void GameObject17::deactivateObject()
{
    GameObject::deactivateObject();
    gameObj->obf_7C->playSound("step_switch.wav");
}

// AS: GameObject17.stepOnObject — GameObject's (only while it is up); stepping on it presses it.
void GameObject17::stepOnObject(Character &c)
{
    if (!isDeactivated && !isDeactivating) {
        if (std::find(standingChars.begin(), standingChars.end(), &c) == standingChars.end()) {
            standingChars.push_back(&c);
            standingCharsLastX.push_back(c.x);
        }
        c.jump = false;
        c.onMovingTile = true;
        c.whichMovingTile = this;
    }
    if (isActivated && canReact) {
        deactivateObject();
    }
}

} // namespace papaGame::models::objects
