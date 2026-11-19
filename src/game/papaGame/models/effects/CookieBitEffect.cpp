#include "game/papaGame/models/effects/CookieBitEffect.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: CookieBitEffect constructor — thrown up and to a side.
void CookieBitEffect::init(double x_, double y_, double id_, const std::string &subtype_, bool followPlayer_,
                      double playerXoffset_, double playerYoffset_)
{
    Effect::init(x_, y_, id_, subtype_, followPlayer_, playerXoffset_, playerYoffset_);
    jumpspeed = -3 - std::floor(as3::random() * 8);
    jump = true;
    dirx = 1;
    if (as3::random() > 0.5) {
        dirx = -1;
    }
    speed = 2 + std::floor(as3::random() * 2);
    animationFrame = grabAnimationCycle().frameAt(0);
}

// AS: CookieBitEffect.defineVars
void CookieBitEffect::defineVars()
{
    effectType = "CookieBitEffect";
    sheetname = "effect_cookiebit";
    sheetIsBitmap = false;
    spritewidth = 16;
    spriteheight = 16;
    spriteCenterX = 8;
    spriteCenterY = 8;
    spriteTargetX = 8;
    spriteTargetY = 8;
    sheetWidth = 6;
    sheetHeight = 1;
    gravity = 1;
    normalgravity = 1;
    jumpstart = -14;
    speed = 4;
    foreground = true;
}

// AS: CookieBitEffect.grabAnimationCycle — the piece named by the subtype.
const AnimationCycle &CookieBitEffect::grabAnimationCycle()
{
    return cycleFrames(subtype.empty() ? "piece1CycleFrames" : subtype + "CycleFrames");
}

// AS: CookieBitEffect.endAnimationCycle
void CookieBitEffect::endAnimationCycle()
{
    gameObj->obf_V->removeEffect(id);
}

// AS: CookieBitEffect.updateObject
void CookieBitEffect::updateObject()
{
    if (gameObj->obf_71->checkOnScreen(*this)) {
        jumpChar(false);
        moveChar(dirx, 0, 0, false, false, false);
    } else {
        gameObj->obf_V->removeEffect(id);
    }
}

} // namespace papaGame::models::effects
