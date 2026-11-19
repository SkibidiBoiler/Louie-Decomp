#include "game/papaGame/models/effects/FeatherEffect.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: FeatherEffect constructor — thrown up and to a side.
void FeatherEffect::init(double x_, double y_, double id_, const std::string &subtype_, bool followPlayer_,
                      double playerXoffset_, double playerYoffset_)
{
    Effect::init(x_, y_, id_, subtype_, followPlayer_, playerXoffset_, playerYoffset_);
    jumpspeed = -3 - std::floor(as3::random() * 4);
    jump = true;
    dirx = 1;
    if (as3::random() > 0.5) {
        dirx = -1;
    }
    speed = 1 + std::floor(as3::random() * 2);
    animationFrame = grabAnimationCycle().frameAt(0);
}

// AS: FeatherEffect.defineVars
void FeatherEffect::defineVars()
{
    effectType = "FeatherEffect";
    sheetname = "effect_feather";
    sheetIsBitmap = false;
    spritewidth = 20;
    spriteheight = 20;
    spriteCenterX = 10;
    spriteCenterY = 10;
    spriteTargetX = 10;
    spriteTargetY = 10;
    sheetWidth = 16;
    sheetHeight = 1;
    gravity = 1;
    normalgravity = 1;
    jumpstart = -14;
    speed = 4;
    foreground = true;
    maxjumpvelocity = 6;
}

// AS: FeatherEffect.grabAnimationCycle — the piece named by the subtype (frame 99 is off the sheet: blank).
const AnimationCycle &FeatherEffect::grabAnimationCycle()
{
    return cycleFrames(subtype.empty() ? "piece1CycleFrames" : subtype + "CycleFrames");
}

// AS: FeatherEffect.endAnimationCycle
void FeatherEffect::endAnimationCycle()
{
    gameObj->obf_V->removeEffect(id);
}

// AS: FeatherEffect.updateObject
void FeatherEffect::updateObject()
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
