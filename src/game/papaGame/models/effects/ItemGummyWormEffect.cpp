#include "game/papaGame/models/effects/ItemGummyWormEffect.h"

#include <cmath>

#include "game/obf_AF/obf_AJ.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: ItemGummyWormEffect.defineVars
void ItemGummyWormEffect::defineVars()
{
    effectType = "ItemGummyWormEffect";
    sheetname = "item_gummyworm";
    sheetIsBitmap = false;
    spritewidth = 33;
    spriteheight = 16;
    spriteCenterX = 16;
    spriteCenterY = 8;
    spriteTargetX = 16;
    spriteTargetY = 8;
    sheetWidth = 11;
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

// AS: ItemGummyWormEffect.grabAnimationCycle
const AnimationCycle &ItemGummyWormEffect::grabAnimationCycle()
{
    return cycleFrames("animCycleFrames");
}

// AS: ItemGummyWormEffect.endAnimationCycle
void ItemGummyWormEffect::endAnimationCycle()
{
    gameObj->obf_V->removeEffect(id);
}

// AS: ItemGummyWormEffect.updateObject — eases up distanceToJump pixels, then back down, and is removed.
void ItemGummyWormEffect::updateObject()
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
