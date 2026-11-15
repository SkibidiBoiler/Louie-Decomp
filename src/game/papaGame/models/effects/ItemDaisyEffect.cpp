#include "game/papaGame/models/effects/ItemDaisyEffect.h"

#include <cmath>

#include "game/obf_AF/obf_AJ.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: ItemDaisyEffect.defineVars
void ItemDaisyEffect::defineVars()
{
    effectType = "ItemDaisyEffect";
    sheetname = "item_daisy";
    sheetIsBitmap = false;
    spritewidth = 32;
    spriteheight = 48;
    spriteCenterX = 16;
    spriteCenterY = 24;
    spriteTargetX = 16;
    spriteTargetY = 32;
    sheetWidth = 12;
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

// AS: ItemDaisyEffect.grabAnimationCycle
const AnimationCycle &ItemDaisyEffect::grabAnimationCycle()
{
    return cycleFrames("animCycleFrames");
}

// AS: ItemDaisyEffect.endAnimationCycle
void ItemDaisyEffect::endAnimationCycle()
{
    gameObj->obf_V->removeEffect(id);
}

// AS: ItemDaisyEffect.updateObject — eases up distanceToJump pixels, then back down, and is removed.
void ItemDaisyEffect::updateObject()
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
