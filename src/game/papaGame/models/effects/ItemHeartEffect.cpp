#include "game/papaGame/models/effects/ItemHeartEffect.h"

#include <cmath>

#include "game/obf_AF/obf_AJ.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: ItemHeartEffect.defineVars
void ItemHeartEffect::defineVars()
{
    effectType = "ItemHeartEffect";
    sheetname = "item_heart";
    sheetIsBitmap = false;
    spritewidth = 36;
    spriteheight = 32;
    spriteCenterX = 18;
    spriteCenterY = 16;
    spriteTargetX = 18;
    spriteTargetY = 16;
    sheetWidth = 24;
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

// AS: ItemHeartEffect.grabAnimationCycle
const AnimationCycle &ItemHeartEffect::grabAnimationCycle()
{
    return cycleFrames("animCycleFrames");
}

// AS: ItemHeartEffect.endAnimationCycle
void ItemHeartEffect::endAnimationCycle()
{
    gameObj->obf_V->removeEffect(id);
}

// AS: ItemHeartEffect.updateObject — eases up distanceToJump pixels, then back down, and is removed.
void ItemHeartEffect::updateObject()
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
