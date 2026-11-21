#include "game/papaGame/models/effects/SauceSplatEffect.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: SauceSplatEffect.defineVars
void SauceSplatEffect::defineVars()
{
    effectType = "SauceSplatEffect";
    sheetname = "effect_saucesplat";
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

// AS: SauceSplatEffect.grabAnimationCycle
const AnimationCycle &SauceSplatEffect::grabAnimationCycle()
{
    return cycleFrames("animCycleFrames");
}

// AS: SauceSplatEffect.endAnimationCycle
void SauceSplatEffect::endAnimationCycle()
{
    gameObj->obf_V->removeEffect(id);
}

// AS: SauceSplatEffect.updateObject
void SauceSplatEffect::updateObject()
{
    updateSprite();
}

} // namespace papaGame::models::effects
