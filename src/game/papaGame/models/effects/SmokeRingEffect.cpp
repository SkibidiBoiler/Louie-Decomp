#include "game/papaGame/models/effects/SmokeRingEffect.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: SmokeRingEffect.defineVars
void SmokeRingEffect::defineVars()
{
    effectType = "SmokeRingEffect";
    sheetname = "effect_smokering";
    sheetIsBitmap = false;
    spritewidth = 56;
    spriteheight = 130;
    spriteCenterX = 28;
    spriteCenterY = 65;
    spriteTargetX = 28;
    spriteTargetY = 65;
    height = 4;
    width = 4;
    collRect = Rect{-4, -4, 8, 8};
    sheetWidth = 15;
    sheetHeight = 1;
    gravity = 1;
    normalgravity = 1;
    jumpstart = -14;
    speed = 4;
    foreground = true;
}

// AS: SmokeRingEffect.grabAnimationCycle
const AnimationCycle &SmokeRingEffect::grabAnimationCycle()
{
    return cycleFrames("animCycleFrames");
}

// AS: SmokeRingEffect.endAnimationCycle
void SmokeRingEffect::endAnimationCycle()
{
    gameObj->obf_V->removeEffect(id);
}

// AS: SmokeRingEffect.updateObject — it sticks to the player when told to.
void SmokeRingEffect::updateObject()
{
    const display::GameDisplay &display = *gameObj->obf_71;
    if (followPlayer) {
        x = gameObj->playerObj->x + playerXoffset;
        y = gameObj->playerObj->y + playerYoffset;
        xtile = std::floor(x / display.tileWidth);
        ytile = std::floor(y / display.tileWidth);
    }
    updateSprite();
}

} // namespace papaGame::models::effects
