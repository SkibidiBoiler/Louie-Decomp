#include "game/papaGame/models/items/GameItem_FizzoCan.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::items {

// AS: GameItem_FizzoCan.defineVars
void GameItem_FizzoCan::defineVars()
{
    sheetname = "item_fizzocan";
    sheetIsBitmap = false;
    itemType = "_FizzoCan";
    spritewidth = 15;
    spriteheight = 29;
    spriteCenterX = 7;
    spriteCenterY = 14;
    spriteTargetX = 7;
    spriteTargetY = 13;
    height = 8;
    width = 8;
    collRect = {-7, -12, 14, 24};
    sheetWidth = 1;
    sheetHeight = 1;
    jumpstart = 0;
    gravity = 0;
    normalgravity = 0;
    speed = 0;
    bounceY = false;
    foreground = false;
    points = 10;
    moneyValue = 1;
    animCycleFrames = cycleType2("anim", 1, 0, 0, {0});
    flickerCycleFrames = cycleType2("flicker", 1, 0, 0, {0, 99});
    doesMove = false;
    doesDisappear = false;
    shouldSaveOnRoomChange = true;
}

// AS: GameItem_FizzoCan.collectItem
void GameItem_FizzoCan::collectItem()
{
    gameObj->obf_51->collectSpecialItem();
    gameObj->obf_51->earnPoints(points);
    gameObj->obf_V->addEffect(0, 0, "ItemFizzoCanEffect", "", true, 0, -50);
    gameObj->obf_7C->playSound("grabitem.wav");
    gameObj->obf_3Z->removeItem(id);
}

} // namespace papaGame::models::items
