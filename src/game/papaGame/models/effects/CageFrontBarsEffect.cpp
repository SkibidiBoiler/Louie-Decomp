#include "game/papaGame/models/effects/CageFrontBarsEffect.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: CageFrontBarsEffect.defineVars
void CageFrontBarsEffect::defineVars()
{
    effectType = "CageFrontBarsEffect";
    sheetname = "effect_cagefrontbars";
    sheetIsBitmap = false;
    spritewidth = 128;
    spriteheight = 192;
    spriteCenterX = 64;
    spriteCenterY = 96;
    spriteTargetX = 16;
    spriteTargetY = 16;
    sheetWidth = 22;
    sheetHeight = 1;
    gravity = 1;
    normalgravity = 1;
    jumpstart = -4;
    speed = 2;
    foreground = true;
}

// AS: CageFrontBarsEffect.grabAnimationCycle
const AnimationCycle &CageFrontBarsEffect::grabAnimationCycle()
{
    return cycleFrames(isOpen ? "openCycleFrames" : "closedCycleFrames");
}

// AS: CageFrontBarsEffect.updateObject
void CageFrontBarsEffect::updateObject()
{
    if (gameObj->obf_71->checkOnScreen(*this)) {
        updateSprite();
    }
}

// AS: CageFrontBarsEffect.checkOnScreen
bool CageFrontBarsEffect::checkOnScreen() const
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 5 && xtile < d.currentXtileRight + 5;
}

} // namespace papaGame::models::effects
