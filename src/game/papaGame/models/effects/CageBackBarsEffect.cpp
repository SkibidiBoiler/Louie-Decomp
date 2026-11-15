#include "game/papaGame/models/effects/CageBackBarsEffect.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: CageBackBarsEffect.defineVars
void CageBackBarsEffect::defineVars()
{
    effectType = "CageBackBarsEffect";
    sheetname = "effect_cagebackbars";
    sheetIsBitmap = false;
    spritewidth = 128;
    spriteheight = 128;
    spriteCenterX = 64;
    spriteCenterY = 64;
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

// AS: CageBackBarsEffect.grabAnimationCycle
const AnimationCycle &CageBackBarsEffect::grabAnimationCycle()
{
    return cycleFrames(isOpen ? "openCycleFrames" : "closedCycleFrames");
}

// AS: CageBackBarsEffect.updateObject
void CageBackBarsEffect::updateObject()
{
    if (gameObj->obf_71->checkOnScreen(*this)) {
        updateSprite();
    }
}

// AS: CageBackBarsEffect.checkOnScreen
bool CageBackBarsEffect::checkOnScreen() const
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 5 && xtile < d.currentXtileRight + 5;
}

} // namespace papaGame::models::effects
