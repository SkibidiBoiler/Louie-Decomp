#include "game/papaGame/models/effects/EndPortalEffect.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: EndPortalEffect.defineVars
void EndPortalEffect::defineVars()
{
    effectType = "EndPortalEffect";
    sheetname = "effect_endportal";
    sheetIsBitmap = false;
    spritewidth = 106;
    spriteheight = 106;
    spriteCenterX = 53;
    spriteCenterY = 53;
    spriteTargetX = 53;
    spriteTargetY = 53;
    sheetWidth = 14;
    sheetHeight = 1;
    foreground = false;
}

// AS: EndPortalEffect.grabAnimationCycle
const AnimationCycle &EndPortalEffect::grabAnimationCycle()
{
    if (subtype == "startup") {
        return cycleFrames("startupCycleFrames");
    }
    if (isClosed) {
        return cycleFrames("closeCycleFrames");
    }
    if (subtype == "delay") {
        return cycleFrames("delayCycleFrames");
    }
    return cycleFrames("animCycleFrames");
}

// AS: EndPortalEffect.endAnimationCycle
void EndPortalEffect::endAnimationCycle()
{
    gameObj->obf_V->removeEffect(id);
}

// AS: EndPortalEffect.updateObject
void EndPortalEffect::updateObject()
{
    updateSprite();
}

// AS: EndPortalEffect.checkOnScreen
bool EndPortalEffect::checkOnScreen() const
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 5 && xtile < d.currentXtileRight + 5;
}

} // namespace papaGame::models::effects
