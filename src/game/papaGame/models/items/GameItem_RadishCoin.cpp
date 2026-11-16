#include "game/papaGame/models/items/GameItem_RadishCoin.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::items {

// AS: GameItem_RadishCoin.defineVars
void GameItem_RadishCoin::defineVars()
{
    sheetname = "item_radcoin";
    sheetIsBitmap = false;
    itemType = "_RadishCoin";
    spritewidth = 50;
    spriteheight = 60;
    spriteCenterX = 25;
    spriteCenterY = 30;
    spriteTargetX = 25;
    spriteTargetY = 35;
    height = 8;
    width = 8;
    collRect = {-12, -12, 24, 24};
    sheetWidth = 28;
    sheetHeight = 1;
    jumpstart = 0;
    gravity = 0;
    normalgravity = 0;
    speed = 0;
    bounceY = false;
    foreground = false;
    points = 10;
    moneyValue = 1;
    animCycleFrames = cycleType2("anim", 1, 0, 0, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27});
    flickerCycleFrames = cycleType2("flicker", 1, 0, 0, {5, 99});
    doesMove = false;
    doesDisappear = false;
    shouldSaveOnRoomChange = true;
}

// AS: GameItem_RadishCoin.collectItem
void GameItem_RadishCoin::collectItem()
{
    gameObj->obf_51->collectSpecialItem();
    gameObj->obf_51->earnPoints(points);
    gameObj->obf_V->addEffect(0, 0, "ItemRadishCoinEffect", "", true, 0, -50);
    gameObj->obf_7C->playSound("grabitem.wav");
    gameObj->obf_3Z->removeItem(id);
}

} // namespace papaGame::models::items
