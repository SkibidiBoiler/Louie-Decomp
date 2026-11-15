#include "game/papaGame/models/objects/GameObject28.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::objects {

// AS: GameObject28.defineVars
void GameObject28::defineVars()
{
    sheetname = "object_checkpoint";
    objectname = "Checkpoint";
    sheetIsBitmap = false;
    type = 28;
    spritewidth = 50;
    spriteheight = 87;
    spriteCenterX = 25;
    spriteCenterY = 43;
    spriteTargetX = 25;
    spriteTargetY = 70;
    sheetWidth = 17;
    sheetHeight = 1;
    collRect = {-20, -27, 40, 40};
    width = 15;
    height = 15;
    playerheight = 15;
    activeCycleFrames = cycleType2("active", 1, 0, 0, {0, 1, 2, 3, 4, 5, 6, 5, 4, 3, 2, 1});
    deactivatingCycleFrames = cycleType2("deactivating", 1, 0, -1, {7});
    inactiveCycleFrames = cycleType2("inactive", 1, 0, 23, {8,  9,  10, 11, 12, 13, 14, 15, 11, 12, 13, 14,
                                                            15, 11, 12, 13, 14, 15, 11, 12, 13, 14, 15, 16});
    activatingCycleFrames = cycleType2("activating", 1, 0, -1, {0});
    isActivated = true;
    currentHealth = 3;
    maxHealth = 3;
    foreground = false;
    isSteppable = false;
    isSteppableByObjects = false;
    broadcastStyle = 1;
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

// AS: GameObject28.setupParams — [checkpoint id, saveState, active]
void GameObject28::setupParams(const std::vector<as3::Value> &params)
{
    if (params.size() > 0) {
        checkpointID = params[0].asNumber();
    }
    if (params.size() > 1) {
        saveState = params[1].asNumber() == 1;
    }
    if (params.size() > 2 && params[2].asNumber() != 1) {
        isActivated = false;
        isDeactivated = true;
    } else {
        isActivated = true;
        isDeactivated = false;
    }
}

// AS: GameObject28.updateObject — the player touching an active checkpoint turns it.
void GameObject28::updateObject()
{
    if (isActivated && !isDeactivated && !isDeactivating) {
        const Rect r{x + collRect.x, y + collRect.y, collRect.width, collRect.height};
        if (gameObj->playerObj->checkSpriteCollision(r)) {
            deactivateObject();
        }
    }
    updateSprite();
}

// AS: GameObject28.finishDeactivatingObject
void GameObject28::finishDeactivatingObject()
{
    GameObject::finishDeactivatingObject();
    gameObj->obf_AQ->saveCheckpoint(xtile, ytile - 1);
    gameObj->playerObj->currentHealth = gameObj->playerObj->maxHealth;
    // SKELETON: GameHUD.updateDisplay / updatePlayerHealth.
    const auto &p = *gameObj->playerObj;
    for (int i = 1; i <= 6; ++i) {
        gameObj->obf_V->addEffect(p.x, p.y, "PlayerCheckpointEffect", "star" + std::to_string(i), true, 0, -32);
    }
    // SKELETON: checkpoint.wav.
}

// AS: GameObject28.saveObjectData
void GameObject28::saveObjectData()
{
    std::vector<as3::Value> row{as3::Value::num(type), as3::Value::num(xtile), as3::Value::num(ytile),
                                as3::Value::num(currentTriggers)};
    row.push_back(as3::Value::num(checkpointID));
    row.push_back(as3::Value::num(saveState ? 1 : 0));
    row.push_back(as3::Value::num(isActivated && !isDeactivated && !isDeactivating ? 1 : 0));
    gameObj->obf_AQ->currentScreenData->saveObjectState(id - 1, row);
}

} // namespace papaGame::models::objects
