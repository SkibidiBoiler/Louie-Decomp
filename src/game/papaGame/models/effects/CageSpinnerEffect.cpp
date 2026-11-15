#include "game/papaGame/models/effects/CageSpinnerEffect.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: CageSpinnerEffect.defineVars
void CageSpinnerEffect::defineVars()
{
    effectType = "CageSpinnerEffect";
    sheetname = "effect_cagespinner";
    sheetIsBitmap = false;
    spritewidth = 41;
    spriteheight = 41;
    spriteCenterX = 20;
    spriteCenterY = 20;
    spriteTargetX = 20;
    spriteTargetY = 20;
    sheetWidth = 28;
    sheetHeight = 1;
    gravity = 1;
    normalgravity = 1;
    jumpstart = -4;
    speed = 2;
    foreground = true;
}

// AS: CageSpinnerEffect.grabAnimationCycle
const AnimationCycle &CageSpinnerEffect::grabAnimationCycle()
{
    return cycleFrames(isOpen ? "openCycleFrames" : "closedCycleFrames");
}

// AS: CageSpinnerEffect.updateObject
void CageSpinnerEffect::updateObject()
{
    if (gameObj->obf_71->checkOnScreen(*this)) {
        updateSprite();
    }
}

// AS: CageSpinnerEffect.checkOnScreen
bool CageSpinnerEffect::checkOnScreen() const
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 3 && xtile < d.currentXtileRight + 3;
}

} // namespace papaGame::models::effects
