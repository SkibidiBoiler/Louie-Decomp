#include "game/papaGame/models/objects/GameObject1.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::objects {

// AS: GameObject1.defineVars
void GameObject1::defineVars()
{
    sheetname = "object_cagepulley";
    objectname = "Cage Pulley";
    sheetIsBitmap = false;
    type = 1;
    spritewidth = 104;
    spriteheight = 184;
    spriteCenterX = 52;
    spriteCenterY = 92;
    spriteTargetX = 16;
    spriteTargetY = 22;
    sheetWidth = 30;
    sheetHeight = 1;
    collRect = {-16, -16, 104, 60};
    activeCycleFrames = cycleType1("active", 1, 0, 0, 0, 0);
    inactiveCycleFrames = cycleType2("inactive", 1, 0, 27, {2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
                                                            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29});
    activatingCycleFrames = cycleType2("activating", 1, 0, -1, {28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15,
                                                                 14, 13, 12, 11, 10, 9,  8,  7,  6,  5,  4,  3,  2,  1});
    deactivatingCycleFrames = cycleType2("deactivating", 1, 0, -1, {1});
    isActivated = true;
    currentHealth = 3;
    maxHealth = 3;
    foreground = false;
    flipBlitOffset = 0;
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
    heightmultiplier = 3;
    widthmultiplier = 3;
    speed = 0;
    walkspeed = 0;
    isGrabRideable = true;
    delayRideableUntilPickup = false;
    rideablePlayerOffsetX = 39;
    rideablePlayerOffsetY = 116;
    canJumpFromRideable = false;
    isSolidActive = true;
    isSolidInactive = true;
    solidTilesRect = {0, 0, 3, 1};
    solidType = "collision";
}

// AS: GameObject1.setupParams (no parameters are read)
void GameObject1::setupParams(const std::vector<as3::Value> & /*params*/) {}

// AS: GameObject1.grabAnimationCycle
const AnimationCycle &GameObject1::grabAnimationCycle()
{
    return GameObject::grabAnimationCycle();
}

// AS: GameObject1.checkOnScreen
bool GameObject1::checkOnScreen()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 5 && xtile < d.currentXtileRight + 5;
}

// AS: GameObject1.updateObject — carries the rider down the rope (grabYextraOffsets by cycle frame).
void GameObject1::updateObject()
{
    static constexpr double grabYextraOffsets[] = {0,  1,  2,  4,  6,  9,  12, 16,  20,  25,  31,  37,  43,  50,  57,
                                                   63, 69, 74, 79, 84, 88, 91, 94, 97, 99, 101, 102, 103, 103, 103};
    if (isHit) {
        return;
    }
    updateSprite();
    if (animationFrame == 29 && isGrabRideable) {
        releaseRideableObject();
        isGrabRideable = false;
    }
    if (isGrabRiding && grabbingChar) {
        const double tw = gameObj->obf_71->tileWidth;
        const double dx = x + rideablePlayerOffsetX - grabbingChar->x;
        grabbingChar->x += dx / 2;
        if (std::abs(dx) <= 1) {
            grabbingChar->x = x + rideablePlayerOffsetX;
        }
        const auto frame = static_cast<size_t>(cycleFrame);
        grabbingChar->y = y + rideablePlayerOffsetY + (frame < std::size(grabYextraOffsets) ? grabYextraOffsets[frame] : 0);
        grabbingChar->xtile = std::floor(grabbingChar->x / tw);
        grabbingChar->ytile = std::floor(grabbingChar->y / tw);
    }
}

// AS: GameObject1.grabRideableObject — "Player Grab Pulley!"
void GameObject1::grabRideableObject(Character *c)
{
    isGrabRiding = true;
    grabbingChar = c;
    deactivateObject();
}

// AS: GameObject1.releaseRideableObject — "Player Release Pulley!"
void GameObject1::releaseRideableObject()
{
    isGrabRiding = false;
    if (grabbingChar && grabbingChar->isPlayer()) {
        auto *p = static_cast<characters::CustomerChar *>(grabbingChar);
        p->isRidingObject = false;
        p->whichObjectRiding = nullptr;
    }
    if (grabbingChar) {
        grabbingChar->jump = true;
        grabbingChar->jumpspeed = 0;
    }
    grabbingChar = nullptr;
}

// AS: GameObject1.saveObjectData
void GameObject1::saveObjectData()
{
    const std::vector<as3::Value> row{as3::Value::num(type), as3::Value::num(xtile), as3::Value::num(ytile),
                                      as3::Value::num(currentTriggers)};
    gameObj->obf_AQ->currentScreenData->saveObjectState(id - 1, row, true);
}

} // namespace papaGame::models::objects
