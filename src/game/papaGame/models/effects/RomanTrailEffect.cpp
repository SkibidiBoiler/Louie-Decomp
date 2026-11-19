#include "game/papaGame/models/effects/RomanTrailEffect.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: RomanTrailEffect.defineVars
void RomanTrailEffect::defineVars()
{
    effectType = "RomanTrailEffect";
    sheetname = "effect_romantrail";
    sheetIsBitmap = false;
    spritewidth = 24;
    spriteheight = 24;
    spriteCenterX = 12;
    spriteCenterY = 12;
    spriteTargetX = 12;
    spriteTargetY = 12;
    height = 4;
    width = 4;
    collRect = Rect{-4, -4, 8, 8};
    sheetWidth = 4;
    sheetHeight = 1;
    gravity = 1;
    normalgravity = 1;
    jumpstart = -14;
    speed = 4;
    foreground = true;
}

// AS: RomanTrailEffect.endAnimationCycle
void RomanTrailEffect::endAnimationCycle()
{
    gameObj->obf_V->removeEffect(id);
}

} // namespace papaGame::models::effects
