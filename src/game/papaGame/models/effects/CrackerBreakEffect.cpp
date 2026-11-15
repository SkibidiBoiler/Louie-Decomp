#include "game/papaGame/models/effects/CrackerBreakEffect.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: CrackerBreakEffect constructor — thrown up and to a side.
void CrackerBreakEffect::init(double x_, double y_, double id_, const std::string &subtype_, bool followPlayer_,
                      double playerXoffset_, double playerYoffset_)
{
    Effect::init(x_, y_, id_, subtype_, followPlayer_, playerXoffset_, playerYoffset_);
    jumpspeed = -8 - std::floor(as3::random() * 8);
    jump = true;
    dirx = 1;
    if (as3::random() > 0.5) {
        dirx = -1;
    }
    speed = 2 + std::floor(as3::random() * 4);
    animationFrame = grabAnimationCycle().frameAt(0);
}

// AS: CrackerBreakEffect.defineVars
void CrackerBreakEffect::defineVars()
{
    effectType = "CrackerBreakEffect";
    sheetname = "effect_crackersmash";
    sheetIsBitmap = false;
    spritewidth = 22;
    spriteheight = 22;
    spriteCenterX = 11;
    spriteCenterY = 11;
    spriteTargetX = 11;
    spriteTargetY = 11;
    sheetWidth = 6;
    sheetHeight = 1;
    gravity = 1;
    normalgravity = 1;
    jumpstart = -14;
    speed = 4;
    foreground = true;
}

// AS: CrackerBreakEffect.grabAnimationCycle — the piece named by the subtype.
const AnimationCycle &CrackerBreakEffect::grabAnimationCycle()
{
    return cycleFrames(subtype.empty() ? "piece1CycleFrames" : subtype + "CycleFrames");
}

// AS: CrackerBreakEffect.endAnimationCycle
void CrackerBreakEffect::endAnimationCycle()
{
    gameObj->obf_V->removeEffect(id);
}

// AS: CrackerBreakEffect.updateObject
void CrackerBreakEffect::updateObject()
{
    if (gameObj->obf_71->checkOnScreen(*this)) {
        jumpChar(false);
        moveChar(dirx, 0, 0, false, false, false);
        updateSprite();
    } else {
        gameObj->obf_V->removeEffect(id);
    }
}

} // namespace papaGame::models::effects
