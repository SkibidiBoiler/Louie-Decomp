#include "game/papaGame/models/objects/GameObject18.h"

#include <SDL3/SDL.h>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::objects {

// AS: GameObject18.GameObject18 — its front is drawn as an effect.
void GameObject18::init(double xtile_, double ytile_, double id_, double whichScreen_, double screenObjID_,
                        const std::vector<as3::Value> &extraParams)
{
    GameObject::init(xtile_, ytile_, id_, whichScreen_, screenObjID_, extraParams);
    frontEffectID = gameObj->obf_V->addEffect(x, y, "DoorFrontEffect");
}

// AS: GameObject18.defineVars
void GameObject18::defineVars()
{
    sheetname = "object_door";
    sheetIsBitmap = false;
    objectname = "Door";
    type = 18;
    spritewidth = 50;
    spriteheight = 136;
    spriteCenterX = 25;
    spriteCenterY = 68;
    spriteTargetX = 16;
    spriteTargetY = 16;
    sheetWidth = 11;
    sheetHeight = 1;
    collRect = {-26, -26, 52, 52};
    activeCycleFrames = cycleType1("active", 1, 0, 10, 10, 10);
    inactiveCycleFrames = cycleType1("inactive", 1, 0, 0, 0, 0);
    activatingCycleFrames = cycleType2("activating", 1, 0, -1, {1, 2, 3, 4, 5, 6, 7, 8, 9});
    deactivatingCycleFrames = cycleType2("deactivating", 1, 0, -1, {9, 8, 7, 6, 5, 4, 3, 2, 1});
    isSolidActive = false;
    isSolidInactive = true;
    solidTilesRect = {0, 0, 1, 4};
    solidType = "collision";
    isShootable = false;
    isActivated = true;
    isDeactivated = false;
    currentHealth = 3;
    maxHealth = 3;
    foreground = false;
    broadcastStyle = 2;
    broadcastOn = true;
    broadcastOff = false;
    broadcastAmountOn = 1;
    broadcastAmountOff = -1;
    broadcastReceiptMethod = 1;
    jigglecam = true;
    jiggleamount = 6;
    jiggleduration = 8;
}

// AS: GameObject18.saveObjectData — QUIRK: currentTriggers is saved twice.
void GameObject18::saveObjectData()
{
    std::vector<as3::Value> row{as3::Value::num(type), as3::Value::num(xtile), as3::Value::num(ytile),
                                as3::Value::num(currentTriggers)};
    row.push_back(as3::Value::num(isActivated ? 1 : 0));
    row.push_back(as3::Value::num(saveState ? 1 : 0));
    row.push_back(as3::Value::num(broadcastReceiptMethod));
    row.push_back(as3::Value::num(currentTriggers));
    gameObj->obf_AQ->currentScreenData->saveObjectState(id - 1, row);
}

// AS: GameObject18.setupParams — [open (1) or shut, saveState, broadcastReceiptMethod, currentTriggers]
void GameObject18::setupParams(const std::vector<as3::Value> &params)
{
    if (params.empty()) {
        currentTriggers = 0;
        broadcastReceiptMethod = 1;
        return;
    }
    if (params[0].asNumber() == 1) {
        isActivated = true;
        isDeactivated = false;
    } else {
        isActivated = false;
        isDeactivated = true;
    }
    if (params.size() > 1) {
        saveState = params[1].asNumber() == 1;
    }
    broadcastReceiptMethod = params.size() > 2 ? params[2].asNumber() : 1;
    currentTriggers = params.size() > 3 ? params[3].asNumber() : 0;
}

// AS: GameObject18.grabAnimationCycle
const AnimationCycle &GameObject18::grabAnimationCycle()
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

// AS: GameObject18.checkOnScreen — four columns either side of the screen.
bool GameObject18::checkOnScreen()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 4 && xtile < d.currentXtileRight + 4;
}

// AS: GameObject18.updateObject — a shut door is drawn over a player on its left; its front is drawn over the
// player except while they are right of a shut door.
void GameObject18::updateObject()
{
    if (isHit) {
        return;
    }
    updateSprite();
    models::Effect *front = gameObj->obf_V->getEffect(frontEffectID);
    if (!front) {
        SDL_Log("ERRR: no door front"); // AS: the error caught by its try/catch
        return;
    }
    if (isDeactivated && gameObj->playerObj->x < x) {
        foreground = true;
        front->foreground = true;
    } else if (isDeactivated && gameObj->playerObj->x >= x) {
        foreground = false;
        front->foreground = false;
    } else {
        foreground = false;
        front->foreground = true;
    }
}

// AS: GameObject18.destroy
void GameObject18::destroy()
{
    if (frontEffectID != -1) {
        gameObj->obf_V->removeEffect(frontEffectID);
    }
    GameObject::destroy();
}

// AS: GameObject18.finishDeactivatingObject
void GameObject18::finishDeactivatingObject()
{
    GameObject::finishDeactivatingObject();
    gameObj->obf_7C->playSound("papow.wav");
}

// AS: GameObject18.finishActivatingObject
void GameObject18::finishActivatingObject()
{
    GameObject::finishActivatingObject();
    gameObj->obf_7C->playSound("papow.wav");
}

} // namespace papaGame::models::objects
