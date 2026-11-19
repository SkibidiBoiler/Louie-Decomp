#include "game/papaGame/models/effects/ExplosionEffect.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: ExplosionEffect.defineVars
void ExplosionEffect::defineVars()
{
    effectType = "ExplosionEffect";
    sheetname = "effect_explosion";
    sheetIsBitmap = false;
    spritewidth = 130;
    spriteheight = 95;
    spriteCenterX = 65;
    spriteCenterY = 47;
    spriteTargetX = 65;
    spriteTargetY = 70;
    height = 4;
    width = 4;
    collRect = Rect{-4, -4, 8, 8};
    sheetWidth = 21;
    sheetHeight = 1;
    gravity = 1;
    normalgravity = 1;
    jumpstart = -14;
    speed = 4;
    foreground = true;
}

// AS: ExplosionEffect.endAnimationCycle
void ExplosionEffect::endAnimationCycle()
{
    gameObj->obf_V->removeEffect(id);
}

} // namespace papaGame::models::effects
