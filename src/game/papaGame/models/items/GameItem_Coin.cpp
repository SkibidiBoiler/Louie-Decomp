#include "game/papaGame/models/items/GameItem_Coin.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::items {

// AS: GameItem_Coin.defineVars
void GameItem_Coin::defineVars()
{
    sheetname = "item_coin";
    sheetIsBitmap = false;
    itemType = "_Coin";
    spritewidth = 32;
    spriteheight = 32;
    spriteCenterX = 16;
    spriteCenterY = 16;
    spriteTargetX = 16;
    spriteTargetY = 16;
    height = 8;
    width = 8;
    collRect = {-8, -8, 16, 16};
    sheetWidth = 24;
    sheetHeight = 1;
    jumpstart = 0;
    gravity = 0;
    normalgravity = 0;
    speed = 0;
    bounceY = false;
    foreground = false;
    points = 5;
    moneyValue = 1;
    animCycleFrames = cycleType2("anim", 1, 0, 0, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23});
    flickerCycleFrames = cycleType2("flicker", 1, 0, 0, {5, 99});
    doesMove = false;
    doesDisappear = false;
    shouldSaveOnRoomChange = true;
}

// AS: GameItem_Coin.collectItem
void GameItem_Coin::collectItem()
{
    gameObj->obf_51->earnMoney(moneyValue);
    gameObj->obf_51->earnPoints(points);
    // SKELETON: ChallengeManager.recordCoin (and recordTag("georgito") for Georgito).
    gameObj->obf_V->addEffect(0, 0, "ItemCoinEffect", "", true, 0, -50);
    gameObj->obf_7C->playSound("getcoin.wav");
    gameObj->obf_3Z->removeItem(id);
}

} // namespace papaGame::models::items
