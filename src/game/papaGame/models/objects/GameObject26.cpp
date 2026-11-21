#include "game/papaGame/models/objects/GameObject26.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::objects {

// AS: GameObject26.defineVars
void GameObject26::defineVars()
{
    sheetname = "object_cheesecannon";
    objectname = "Cheese Cannon";
    sheetIsBitmap = false;
    type = 26;
    spritewidth = 56;
    spriteheight = 86;
    spriteCenterX = 28;
    spriteCenterY = 43;
    spriteTargetX = 28;
    spriteTargetY = 68;
    sheetWidth = 7;
    sheetHeight = 1;
    collRect = {-20, -27, 40, 50};
    width = 15;
    height = 15;
    playerheight = 15;
    activeCycleFrames = cycleType2("active", 1, 0, 0, {0});
    inactiveCycleFrames = cycleType1("inactive", 1, 0, 0, 0, 0);
    activatingCycleFrames = cycleType2("activating", 1, 0, -1, {0});
    deactivatingCycleFrames = cycleType2("deactivating", 1, 0, -1, {0});
    isActivated = true;
    currentHealth = 3;
    maxHealth = 3;
    foreground = false;
    isSteppable = false;
    isSteppableByObjects = false;
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

// AS: GameObject26.setupParams — [frames between shots, starting timer].
void GameObject26::setupParams(const std::vector<as3::Value> &params)
{
    if (params.size() > 0) {
        shootDelay = params[0].asNumber();
    }
    if (params.size() > 1) {
        mainTimer = params[1].asNumber();
        startedMainTimer = params[1].asNumber();
    }
}

// AS: GameObject26.saveObjectData
void GameObject26::saveObjectData()
{
    std::vector<as3::Value> row{as3::Value::num(type), as3::Value::num(xtile), as3::Value::num(ytile),
                                as3::Value::num(currentTriggers)};
    row.push_back(as3::Value::num(shootDelay));
    row.push_back(as3::Value::num(startedMainTimer));
    gameObj->obf_AQ->currentScreenData->saveObjectState(id - 1, row);
}

// AS: GameObject26.destroy
void GameObject26::destroy()
{
    saveObjectData();
}

// AS: GameObject26.grabAnimationCycle
const AnimationCycle &GameObject26::grabAnimationCycle()
{
    if (isGettingReady) {
        return readyCycleFrames;
    }
    if (isShooting) {
        return shootCycleFrames;
    }
    return staticCycleFrames;
}

// AS: GameObject26.endAnimationCycle — it fires at the end of the wind-up.
void GameObject26::endAnimationCycle()
{
    if (cycleName == "ready") {
        shootCheese();
    } else if (cycleName == "shoot") {
        isShooting = false;
        isGettingReady = false;
        mainTimer = 0;
    }
}

// AS: GameObject26.checkOnScreen — one column either side of the screen.
bool GameObject26::checkOnScreen()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 1 && xtile < d.currentXtileRight + 1;
}

// AS: GameObject26.updateObject — the wind-up starts after shootDelay frames.
void GameObject26::updateObject()
{
    lastx = x;
    lasty = y;
    lastOnSlope = onSlope;
    if (isHit) {
        return;
    }
    if (!isShooting && !isGettingReady) {
        ++mainTimer;
        if (mainTimer >= shootDelay) {
            isGettingReady = true;
        }
    }
    updateSprite();
}

// AS: GameObject26.checkReallyOnScreen
bool GameObject26::checkReallyOnScreen() const
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile >= d.currentXtileLeft - 1 && xtile <= d.currentXtileRight + 1 && ytile >= d.currentYtileUp - 1 &&
           ytile <= d.currentYtileDown + 1;
}

// AS: GameObject26.shootCheese — off screen it goes through the motion without a shot.
void GameObject26::shootCheese()
{
    isGettingReady = false;
    isShooting = true;
    if (!checkReallyOnScreen()) {
        return;
    }
    const double dir = gameObj->playerObj->x < x ? -1 : 1;
    const double sx = x + 30 * dir;
    const double sy = y - 44;
    gameObj->obf_4l->addBullet(sx, sy, dir, 0, "CheeseBullet", "", true, 1, true);
    gameObj->obf_7C->playSound("battle_bow1.wav");
    managers::EffectManager &fx = *gameObj->obf_V;
    fx.addEffect(sx, sy, "CheeseBitEffect", "piece1");
    fx.addEffect(sx - 3, sy - 1, "CheeseBitEffect", "piece2");
    fx.addEffect(sx + 3, sy - 1, "CheeseBitEffect", "piece2");
    fx.addEffect(sx - 4, sy, "CheeseBitEffect", "piece3");
    fx.addEffect(sx + 4, sy, "CheeseBitEffect", "piece3");
    fx.addEffect(sx, sy, "CheeseBitEffect", "piece3");
    fx.addEffect(sx, sy, "CheeseBitEffect", "piece3");
}

} // namespace papaGame::models::objects
