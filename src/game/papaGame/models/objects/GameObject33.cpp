#include "game/papaGame/models/objects/GameObject33.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::objects {

// AS: GameObject33.defineVars
void GameObject33::defineVars()
{
    sheetname = "blanksprite_sheet";
    objectname = "Training Sensor";
    type = 33;
    spritewidth = 64;
    spriteheight = 64;
    spriteCenterX = 32;
    spriteCenterY = 32;
    spriteTargetX = 16;
    spriteTargetY = 16;
    sheetWidth = 1;
    sheetHeight = 1;
    collRect = {-16, -16, 64, 64};
    isActivated = true;
    foreground = true;
    broadcastStyle = 1;
    broadcastAmountOff = 1;
}

// AS: GameObject33.saveObjectData
void GameObject33::saveObjectData()
{
    std::vector<as3::Value> row{as3::Value::num(type), as3::Value::num(xtile), as3::Value::num(ytile),
                                as3::Value::num(currentTriggers)};
    row.push_back(as3::Value::str(trainingFlag));
    row.push_back(as3::Value::str(key1));
    row.push_back(as3::Value::str(key2));
    row.push_back(as3::Value::str(key3));
    row.push_back(as3::Value::num(timeout));
    row.push_back(as3::Value::str(triggerMode));
    row.push_back(as3::Value::str(skillType));
    row.push_back(as3::Value::num(isActivated ? 1 : 0));
    gameObj->obf_AQ->currentScreenData->saveObjectState(id - 1, row);
}

// AS: GameObject33.setupParams — [flag, key1, key2, key3, timeout, triggerMode ("overlaps" / "onscreen"),
// skillType ("any" when missing), activated].
void GameObject33::setupParams(const std::vector<as3::Value> &params)
{
    if (params.size() > 0) {
        trainingFlag = params[0].asString();
    }
    if (params.size() > 1) {
        key1 = params[1].asString();
    }
    if (params.size() > 2) {
        key2 = params[2].asString();
    }
    if (params.size() > 3) {
        key3 = params[3].asString();
    }
    if (params.size() > 4) {
        timeout = params[4].asNumber();
    }
    if (params.size() > 5) {
        triggerMode = params[5].asString();
    }
    skillType = params.size() > 6 ? params[6].asString() : "any";
    if (params.size() > 7) {
        isActivated = params[7].asNumber() == 1;
        isDeactivated = !isActivated;
    }
}

// AS: GameObject33.checkOnScreen — the screen, two tiles more to the left and above.
bool GameObject33::checkOnScreen()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    if (xtile > d.currentXtileLeft - 2 && xtile < d.currentXtileRight + 1) {
        return ytile > d.currentYtileUp - 2 && ytile < d.currentYtileDown + 1;
    }
    return false;
}

// AS: GameObject33.updateObject
void GameObject33::updateObject()
{
    if (checkOnScreen()) {
        const auto *data = gameObj->playerObj ? gameObj->playerObj->playerData() : nullptr;
        if (skillType == "any" || (data && data->skillType == skillType)) {
            if (isActivated) {
                const Rect r{x + collRect.x, y + collRect.y, collRect.width, collRect.height};
                if (gameObj->playerObj->checkSpriteCollision(r) && triggerMode == "overlaps") {
                    deactivateObject();
                } else if (checkOnScreen() && triggerMode == "onscreen") {
                    deactivateObject();
                }
            }
        }
    } else if (isDeactivated && !didTryToClose) {
        if (gameObj->obf_a && gameObj->obf_a->trainingFlag == trainingFlag) {
            // (The original logs "Basic <flag> training complete." / "Didn't complete training...")
            if (skillType == "any") {
                gameObj->obf_c(trainingFlag);
            } else {
                gameObj->obf_64();
            }
            didTryToClose = true;
        }
    }
    updateSprite();
}

// AS: GameObject33.finishDeactivatingObject — opens the bubble with the names of the keys now bound.
void GameObject33::finishDeactivatingObject()
{
    GameObject::finishDeactivatingObject();
    const data::DataManager &dm = *gameObj->obf_AQ;
    const std::string label1 = key1.empty() ? key1 : dm.getKeyLabel(key1);
    const std::string label2 = key2.empty() ? key2 : dm.getKeyLabel(key2);
    const std::string label3 = key3.empty() ? key3 : dm.getKeyLabel(key3);
    gameObj->obf_6n(trainingFlag, true, label1, label2, label3, timeout);
}

} // namespace papaGame::models::objects
