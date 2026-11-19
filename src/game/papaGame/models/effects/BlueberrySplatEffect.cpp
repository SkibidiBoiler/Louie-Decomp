#include "game/papaGame/models/effects/BlueberrySplatEffect.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: BlueberrySplatEffect.defineVars
void BlueberrySplatEffect::defineVars()
{
    effectType = "BlueberrySplatEffect";
    sheetname = "effect_blueberrysplat";
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

// AS: BlueberrySplatEffect.grabAnimationCycle
const AnimationCycle &BlueberrySplatEffect::grabAnimationCycle()
{
    return cycleFrames("animCycleFrames");
}

// AS: BlueberrySplatEffect.endAnimationCycle
void BlueberrySplatEffect::endAnimationCycle()
{
    gameObj->obf_V->removeEffect(id);
}

// AS: BlueberrySplatEffect.updateObject
void BlueberrySplatEffect::updateObject()
{
    updateSprite();
}

} // namespace papaGame::models::effects
