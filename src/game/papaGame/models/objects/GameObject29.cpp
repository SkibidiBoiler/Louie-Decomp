#include "game/papaGame/models/objects/GameObject29.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::objects {

// AS: GameObject29.defineVars
void GameObject29::defineVars()
{
    sheetname = "blanksprite_sheet";
    objectname = "Start Fight";
    type = 29;
    spritewidth = 32;
    spriteheight = 128;
    spriteCenterX = 16;
    spriteCenterY = 64;
    spriteTargetX = 16;
    spriteTargetY = 16;
    sheetWidth = 1;
    sheetHeight = 1;
    collRect = {-16, -16, 32, 128};
    isActivated = true;
    foreground = true;
    broadcastStyle = 1;
    broadcastAmountOff = 1;
}

// AS: GameObject29.saveObjectData
void GameObject29::saveObjectData()
{
    std::vector<as3::Value> row{as3::Value::num(type), as3::Value::num(xtile), as3::Value::num(ytile),
                                as3::Value::num(currentTriggers)};
    row.push_back(as3::Value::num(isActivated ? 1 : 0));
    row.push_back(as3::Value::num(shouldLockCamera ? 1 : 0));
    row.push_back(as3::Value::num(cameraLockX));
    row.push_back(as3::Value::num(cameraLockY));
    row.push_back(as3::Value::num(saveState ? 1 : 0));
    gameObj->obf_AQ->currentScreenData->saveObjectState(id - 1, row);
}

// AS: GameObject29.setupParams — [activated, lock camera, lock x tile, lock y tile, save state].
void GameObject29::setupParams(const std::vector<as3::Value> &params)
{
    if (params.size() > 0) {
        isActivated = params[0].asNumber() == 1;
        isDeactivated = !isActivated;
    }
    if (params.size() > 1) {
        shouldLockCamera = params[1].asNumber() == 1;
    }
    if (params.size() > 2) {
        cameraLockX = params[2].asNumber();
    }
    if (params.size() > 3) {
        cameraLockY = params[3].asNumber();
    }
    if (params.size() > 4) {
        saveState = params[4].asNumber() == 1;
    }
}

// AS: GameObject29.checkOnScreen — only the columns count (4 tiles of margin).
bool GameObject29::checkOnScreen()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 4 && xtile < d.currentXtileRight + 4;
}

// AS: GameObject29.updateObject
void GameObject29::updateObject()
{
    if (checkOnScreen() && isActivated) {
        const Rect r{x + collRect.x, y + collRect.y, collRect.width, collRect.height};
        if (gameObj->playerObj->checkSpriteCollision(r)) {
            deactivateObject();
        }
    }
    updateSprite();
}

// AS: GameObject29.finishDeactivatingObject
void GameObject29::finishDeactivatingObject()
{
    GameObject::finishDeactivatingObject();
    if (shouldLockCamera) {
        gameObj->obf_3Y->setCameraLock(cameraLockX, cameraLockY);
    }
}

} // namespace papaGame::models::objects
