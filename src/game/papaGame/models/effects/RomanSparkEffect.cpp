#include "game/papaGame/models/effects/RomanSparkEffect.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: RomanSparkEffect constructor — thrown up, to the side its subtype names.
void RomanSparkEffect::init(double x_, double y_, double id_, const std::string &subtype_, bool followPlayer_,
                      double playerXoffset_, double playerYoffset_)
{
    Effect::init(x_, y_, id_, subtype_, followPlayer_, playerXoffset_, playerYoffset_);
    jumpspeed = -8 - std::floor(as3::random() * 8);
    jump = true;
    dirx = 1;
    if (subtype.rfind("left", 0) == 0) {
        dirx = -1;
    }
    speed = 2 + std::floor(as3::random() * 2);
    animationFrame = grabAnimationCycle().frameAt(0);
}

// AS: RomanSparkEffect.defineVars
void RomanSparkEffect::defineVars()
{
    effectType = "RomanSparkEffect";
    sheetname = "effect_romanspark";
    sheetIsBitmap = false;
    spritewidth = 10;
    spriteheight = 10;
    spriteCenterX = 5;
    spriteCenterY = 5;
    spriteTargetX = 5;
    spriteTargetY = 5;
    sheetWidth = 7;
    sheetHeight = 1;
    gravity = 1;
    normalgravity = 1;
    jumpstart = -14;
    speed = 4;
    foreground = true;
}

// AS: RomanSparkEffect.grabAnimationCycle — the piece named by the subtype.
const AnimationCycle &RomanSparkEffect::grabAnimationCycle()
{
    return cycleFrames(subtype.empty() ? "right1CycleFrames" : subtype + "CycleFrames");
}

// AS: RomanSparkEffect.endAnimationCycle
void RomanSparkEffect::endAnimationCycle()
{
    gameObj->obf_V->removeEffect(id);
}

// AS: RomanSparkEffect.updateObject
void RomanSparkEffect::updateObject()
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
