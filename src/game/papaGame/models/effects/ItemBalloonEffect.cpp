#include "game/papaGame/models/effects/ItemBalloonEffect.h"

#include <cmath>

#include "game/obf_AF/obf_AJ.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: ItemBalloonEffect.defineVars
void ItemBalloonEffect::defineVars()
{
    effectType = "ItemBalloonEffect";
    sheetname = "item_balloon";
    sheetIsBitmap = false;
    spritewidth = 30;
    spriteheight = 92;
    spriteCenterX = 15;
    spriteCenterY = 46;
    spriteTargetX = 15;
    spriteTargetY = 18;
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

// AS: ItemBalloonEffect.grabAnimationCycle
const AnimationCycle &ItemBalloonEffect::grabAnimationCycle()
{
    return cycleFrames("animCycleFrames");
}

// AS: ItemBalloonEffect.endAnimationCycle
void ItemBalloonEffect::endAnimationCycle()
{
    gameObj->obf_V->removeEffect(id);
}

// AS: ItemBalloonEffect.updateObject — eases up distanceToJump pixels, then back down, and is removed.
void ItemBalloonEffect::updateObject()
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
