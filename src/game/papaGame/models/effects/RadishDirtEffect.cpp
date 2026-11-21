#include "game/papaGame/models/effects/RadishDirtEffect.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: RadishDirtEffect constructor — thrown up and to a side.
void RadishDirtEffect::init(double x_, double y_, double id_, const std::string &subtype_, bool followPlayer_,
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

// AS: RadishDirtEffect.defineVars
void RadishDirtEffect::defineVars()
{
    effectType = "RadishDirtEffect";
    sheetname = "effect_radishdirt";
    sheetIsBitmap = false;
    spritewidth = 8;
    spriteheight = 8;
    spriteCenterX = 4;
    spriteCenterY = 4;
    spriteTargetX = 4;
    spriteTargetY = 4;
    sheetWidth = 3;
    sheetHeight = 1;
    gravity = 1;
    normalgravity = 1;
    jumpstart = -14;
    speed = 4;
    foreground = true;
}

// AS: RadishDirtEffect.grabAnimationCycle — the piece named by the subtype.
const AnimationCycle &RadishDirtEffect::grabAnimationCycle()
{
    return cycleFrames(subtype.empty() ? "piece1CycleFrames" : subtype + "CycleFrames");
}

// AS: RadishDirtEffect.endAnimationCycle
void RadishDirtEffect::endAnimationCycle()
{
    gameObj->obf_V->removeEffect(id);
}

// AS: RadishDirtEffect.updateObject — it falls onto the ground and is gone.
void RadishDirtEffect::updateObject()
{
    if (gameObj->obf_71->checkOnScreen(*this)) {
        jumpChar();
        moveChar(dirx, 0, 0);
        updateSprite();
        if (!jump) {
            gameObj->obf_V->removeEffect(id);
        }
    } else {
        gameObj->obf_V->removeEffect(id);
    }
}

} // namespace papaGame::models::effects
