#include "game/papaGame/models/effects/ItemFizzoCanEffect.h"

#include <cmath>

#include "game/obf_AF/obf_AJ.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: ItemFizzoCanEffect.defineVars
void ItemFizzoCanEffect::defineVars()
{
    effectType = "ItemFizzoCanEffect";
    sheetname = "item_fizzocan";
    sheetIsBitmap = false;
    spritewidth = 15;
    spriteheight = 29;
    spriteCenterX = 7;
    spriteCenterY = 14;
    spriteTargetX = 7;
    spriteTargetY = 13;
    sheetWidth = 1;
    sheetHeight = 1;
    height = 4;
    width = 4;
    collRect = {-4, -4, 8, 8};
    gravity = 1;
    normalgravity = 1;
    jumpstart = -14;
    speed = 4;
    foreground = true;
}

// AS: ItemFizzoCanEffect.grabAnimationCycle
const AnimationCycle &ItemFizzoCanEffect::grabAnimationCycle()
{
    return cycleFrames("animCycleFrames");
}

// AS: ItemFizzoCanEffect.endAnimationCycle
void ItemFizzoCanEffect::endAnimationCycle()
{
    gameObj->obf_V->removeEffect(id);
}

// AS: ItemFizzoCanEffect.updateObject — eases up distanceToJump pixels, then back down, and is removed.
void ItemFizzoCanEffect::updateObject()
{
    double offset = 0;
    bool done = false;
    if (followPlayer) {
        x = gameObj->playerObj->x + playerXoffset;
        y = gameObj->playerObj->y + playerYoffset;
    }
    ++jumpTimer;
    if (goingUp) {
        if (jumpTimer == timePerDirection) {
            offset = distanceToJump;
            goingUp = false;
            jumpTimer = 0;
        } else {
            offset = obf_AF::obf_AJ::obf_5w(jumpTimer, 0, distanceToJump, timePerDirection);
        }
        y -= offset;
    } else {
        if (jumpTimer == timePerDirection) {
            offset = distanceToJump;
            jumpTimer = 0;
            done = true;
        } else {
            offset = obf_AF::obf_AJ::obf_7d(jumpTimer, 0, distanceToJump, timePerDirection);
        }
        y = y - distanceToJump + offset;
    }
    const double tw = gameObj->obf_71->tileWidth;
    xtile = std::floor(x / tw);
    ytile = std::floor(y / tw);
    updateSprite();
    if (done) {
        gameObj->obf_V->removeEffect(id);
    }
}

} // namespace papaGame::models::effects
