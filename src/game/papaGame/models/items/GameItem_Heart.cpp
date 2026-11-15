#include "game/papaGame/models/items/GameItem_Heart.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::items {

// AS: GameItem_Heart.defineVars
void GameItem_Heart::defineVars()
{
    sheetname = "item_heart";
    sheetIsBitmap = false;
    itemType = "_Heart";
    spritewidth = 36;
    spriteheight = 32;
    spriteCenterX = 18;
    spriteCenterY = 16;
    spriteTargetX = 18;
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
    points = 0;
    moneyValue = 1;
    animCycleFrames = cycleType2("anim", 1, 0, 0, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23});
    flickerCycleFrames = cycleType2("flicker", 1, 0, 0, {5, 99});
    doesMove = false;
    doesDisappear = false;
    shouldSaveOnRoomChange = true;
}

// AS: GameItem_Heart.collectItem — the heart is not removed: it shows its empty frame from then on.
void GameItem_Heart::collectItem()
{
    if (!wasCollected) {
        gameObj->playerObj->getHeart();
        gameObj->obf_51->earnPoints(points);
        gameObj->obf_V->addEffect(0, 0, "ItemHeartEffect", "", true, 0, -50);
        // SKELETON: getheart.wav.
        wasCollected = true;
    }
}

// AS: GameItem_Heart.grabAnimationCycle
const AnimationCycle &GameItem_Heart::grabAnimationCycle()
{
    if (isFlickering) {
        return flickerCycleFrames;
    }
    return wasCollected ? collectedCycleFrames : animCycleFrames;
}

} // namespace papaGame::models::items
