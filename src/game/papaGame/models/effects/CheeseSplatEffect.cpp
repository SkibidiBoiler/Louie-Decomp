#include "game/papaGame/models/effects/CheeseSplatEffect.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: CheeseSplatEffect.defineVars
void CheeseSplatEffect::defineVars()
{
    effectType = "CheeseSplatEffect";
    sheetname = "effect_cheesesplat";
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

// AS: CheeseSplatEffect.grabAnimationCycle
const AnimationCycle &CheeseSplatEffect::grabAnimationCycle()
{
    return cycleFrames("animCycleFrames");
}

// AS: CheeseSplatEffect.endAnimationCycle
void CheeseSplatEffect::endAnimationCycle()
{
    gameObj->obf_V->removeEffect(id);
}

// AS: CheeseSplatEffect.updateObject
void CheeseSplatEffect::updateObject()
{
    updateSprite();
}

} // namespace papaGame::models::effects
