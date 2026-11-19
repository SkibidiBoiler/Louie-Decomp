#include "game/papaGame/models/items/GameItem_GoldHelmet.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::items {

// AS: GameItem_GoldHelmet constructor — it starts with a jump.
void GameItem_GoldHelmet::init(double xtile_, double ytile_, double id_, double whichScreen_,
                               const std::vector<as3::Value> &extraParams, bool isDropped_)
{
    GameItem::init(xtile_, ytile_, id_, whichScreen_, extraParams, isDropped_);
    jump = true;
    jumpspeed = jumpstart;
}

// AS: GameItem_GoldHelmet.defineVars
void GameItem_GoldHelmet::defineVars()
{
    sheetname = "item_goldhelmet";
    sheetIsBitmap = false;
    itemType = "_GoldHelmet";
    spritewidth = 50;
    spriteheight = 34;
    spriteCenterX = 25;
    spriteCenterY = 17;
    spriteTargetX = 25;
    spriteTargetY = 18;
    height = 8;
    width = 8;
    collRect = {-12, -12, 24, 24};
    sheetWidth = 28;
    sheetHeight = 1;
    jumpstart = -12;
    gravity = 1;
    normalgravity = 1;
    speed = 0;
    bounceY = true;
    foreground = false;
    points = 10;
    moneyValue = 1;
    animCycleFrames = cycleType2("anim", 1, 0, 0, {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13,
                                                   14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27});
    flickerCycleFrames = cycleType2("flicker", 1, 0, 0, {5, 99});
    doesMove = true;
    doesDisappear = false;
    shouldSaveOnRoomChange = true;
}

// AS: GameItem_GoldHelmet.collectItem
void GameItem_GoldHelmet::collectItem()
{
    gameObj->obf_51->collectSpecialItem();
    gameObj->obf_51->earnPoints(points);
    gameObj->obf_V->addEffect(0, 0, "ItemGoldHelmetEffect", "", true, 0, -50);
    gameObj->obf_7C->playSound("grabitem.wav");
    gameObj->obf_3Z->removeItem(id);
}

// AS: GameItem_GoldHelmet.updateObject — GameItem's, with a clink on each bounce (louder up to the third).
void GameItem_GoldHelmet::updateObject()
{
    const double lastJumpSpeed = jumpspeed;
    if (!isWeapon || isEmpty) {
        ++droppedTimer;
    }
    updateSprite();
    if (doesMove) {
        jumpChar();
        if (dirx != 0) {
            moveChar(dirx, 0, 0, true, true);
        }
        if (!hasBounced && (!jump || (lastJumpSpeed >= 0 && jumpspeed < 0))) {
            hasBounced = true;
        }
        if (jump && lastJumpSpeed >= 0 && jumpspeed < 0) {
            ++whichBounceSound;
            if (whichBounceSound > 3) {
                whichBounceSound = 3;
            }
            gameObj->obf_7C->playSound("goldhelmet_drop" + as3::String(whichBounceSound) + ".wav", false,
                                       getProximityVolume(), getProximityPan());
        }
    }
    if (doesDisappear) {
        if (droppedTimer >= timeBeforeDisappear) {
            gameObj->obf_3Z->removeItem(id);
        } else if (droppedTimer >= timeBeforeFlicker && !isFlickering) {
            isFlickering = true;
        }
    }
}

} // namespace papaGame::models::items
