#include "game/papaGame/models/items/GameItem_GummyWorm.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::items {

// AS: GameItem_GummyWorm.defineVars
void GameItem_GummyWorm::defineVars()
{
    sheetname = "item_gummyworm";
    sheetIsBitmap = false;
    itemType = "_GummyWorm";
    spritewidth = 33;
    spriteheight = 16;
    spriteCenterX = 16;
    spriteCenterY = 8;
    spriteTargetX = 16;
    spriteTargetY = 8;
    height = 8;
    width = 8;
    collRect = {-12, -12, 24, 20};
    sheetWidth = 11;
    sheetHeight = 1;
    jumpstart = -1;
    gravity = 1;
    normalgravity = 1;
    speed = 1;
    bounceY = false;
    foreground = false;
    points = 10;
    moneyValue = 1;
    animCycleFrames = cycleType2("anim", 1, 0, 0, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0});
    flickerCycleFrames = cycleType2("flicker", 1, 0, 0, {0, 99});
    doesMove = true;
    doesDisappear = false;
    shouldSaveOnRoomChange = true;
}

// AS: GameItem_GummyWorm.collectItem
void GameItem_GummyWorm::collectItem()
{
    gameObj->obf_51->collectSpecialItem();
    gameObj->obf_51->earnPoints(points);
    gameObj->obf_V->addEffect(0, 0, "ItemGummyWormEffect", "", true, 0, -50);
    // SKELETON: grabitem.wav.
    gameObj->obf_3Z->removeItem(id);
}

// AS: GameItem_GummyWorm.updateObject — it only moves on the stretching frames.
void GameItem_GummyWorm::updateObject()
{
    GameItem::updateObject();
    if (animationFrame == 0 || animationFrame == 10 || animationFrame == 1 || animationFrame == 9) {
        speed = 0;
    } else {
        speed = 1;
    }
}

} // namespace papaGame::models::items
