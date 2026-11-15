#include "game/papaGame/models/effects/StarburstEffect.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: StarburstEffect.defineVars
void StarburstEffect::defineVars()
{
    effectType = "StarburstEffect";
    sheetname = "effect_starburst";
    sheetIsBitmap = false;
    spritewidth = 80;
    spriteheight = 80;
    spriteCenterX = 40;
    spriteCenterY = 40;
    spriteTargetX = 40;
    spriteTargetY = 40;
    sheetWidth = 19;
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

// AS: StarburstEffect.grabAnimationCycle
const AnimationCycle &StarburstEffect::grabAnimationCycle()
{
    return cycleFrames("animCycleFrames");
}

// AS: StarburstEffect.endAnimationCycle
void StarburstEffect::endAnimationCycle()
{
    gameObj->obf_V->removeEffect(id);
}

// AS: StarburstEffect.updateObject
void StarburstEffect::updateObject()
{
    updateSprite();
}

} // namespace papaGame::models::effects
