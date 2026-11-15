#include "game/papaGame/models/items/GameItem_Balloon.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::items {

// AS: GameItem_Balloon.defineVars
void GameItem_Balloon::defineVars()
{
    sheetname = "item_balloon";
    sheetIsBitmap = false;
    itemType = "_Balloon";
    spritewidth = 30;
    spriteheight = 92;
    spriteCenterX = 15;
    spriteCenterY = 46;
    spriteTargetX = 15;
    spriteTargetY = 18;
    height = 8;
    width = 8;
    collRect = {-12, -12, 24, 24};
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

// AS: GameItem_Balloon.collectItem
void GameItem_Balloon::collectItem()
{
    gameObj->obf_51->collectSpecialItem();
    gameObj->obf_51->earnPoints(points);
    // SKELETON: the ItemBalloonEffect over the player and grabitem.wav.
    gameObj->obf_3Z->removeItem(id);
}

// AS: GameItem_Balloon constructor — `initialY = this.y` runs after the super constructor has placed it.
void GameItem_Balloon::init(double xtile_, double ytile_, double id_, double whichScreen_,
                            const std::vector<as3::Value> &extraParams, bool isDropped_)
{
    GameItem::init(xtile_, ytile_, id_, whichScreen_, extraParams, isDropped_);
    initialY = y;
}

// AS: GameItem_Balloon.updateObject — bobs on a sine wave.
void GameItem_Balloon::updateObject()
{
    ++waveTimer;
    y = std::round(initialY + std::sin(waveTimer / wavePeriod) * waveAmplitude);
    ytile = std::floor(y / gameObj->obf_71->tileWidth);
    updateSprite();
}

} // namespace papaGame::models::items
