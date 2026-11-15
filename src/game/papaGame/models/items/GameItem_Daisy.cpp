#include "game/papaGame/models/items/GameItem_Daisy.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::items {

// AS: GameItem_Daisy.defineVars
void GameItem_Daisy::defineVars()
{
    sheetname = "item_daisy";
    sheetIsBitmap = false;
    itemType = "_Daisy";
    spritewidth = 32;
    spriteheight = 48;
    spriteCenterX = 16;
    spriteCenterY = 24;
    spriteTargetX = 16;
    spriteTargetY = 32;
    height = 8;
    width = 8;
    collRect = {-12, -12, 24, 24};
    sheetWidth = 12;
    sheetHeight = 1;
    jumpstart = 0;
    gravity = 0;
    normalgravity = 0;
    speed = 0;
    bounceY = false;
    foreground = false;
    points = 10;
    moneyValue = 1;
    animCycleFrames = cycleType2("anim", 1, 0, 0, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11});
    flickerCycleFrames = cycleType2("flicker", 1, 0, 0, {5, 99});
    doesMove = false;
    doesDisappear = false;
    shouldSaveOnRoomChange = true;
}

// AS: GameItem_Daisy.collectItem
void GameItem_Daisy::collectItem()
{
    gameObj->obf_51->collectSpecialItem();
    gameObj->obf_51->earnPoints(points);
    // SKELETON: the ItemDaisyEffect over the player and grabitem.wav.
    gameObj->obf_3Z->removeItem(id);
}

} // namespace papaGame::models::items
