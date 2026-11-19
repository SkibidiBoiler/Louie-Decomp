#include "game/papaGame/models/effects/ChiliSplatEffect.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: ChiliSplatEffect.defineVars
void ChiliSplatEffect::defineVars()
{
    effectType = "ChiliSplatEffect";
    sheetname = "effect_chilisplat";
    sheetIsBitmap = false;
    spritewidth = 78;
    spriteheight = 72;
    spriteCenterX = 39;
    spriteCenterY = 36;
    spriteTargetX = 29;
    spriteTargetY = 36;
    height = 4;
    width = 4;
    collRect = Rect{-4, -4, 8, 8};
    sheetWidth = 6;
    sheetHeight = 1;
    gravity = 1;
    normalgravity = 1;
    jumpstart = -14;
    speed = 4;
    foreground = true;
}

// AS: ChiliSplatEffect.grabAnimationCycle
const AnimationCycle &ChiliSplatEffect::grabAnimationCycle()
{
    return cycleFrames("animCycleFrames");
}

// AS: ChiliSplatEffect.endAnimationCycle
void ChiliSplatEffect::endAnimationCycle()
{
    gameObj->obf_V->removeEffect(id);
}

// AS: ChiliSplatEffect.updateObject
void ChiliSplatEffect::updateObject()
{
    updateSprite();
}

} // namespace papaGame::models::effects
