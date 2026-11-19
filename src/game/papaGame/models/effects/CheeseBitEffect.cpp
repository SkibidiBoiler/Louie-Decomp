#include "game/papaGame/models/effects/CheeseBitEffect.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: CheeseBitEffect constructor — thrown up and to a side.
void CheeseBitEffect::init(double x_, double y_, double id_, const std::string &subtype_, bool followPlayer_,
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

// AS: CheeseBitEffect.defineVars
void CheeseBitEffect::defineVars()
{
    effectType = "CheeseBitEffect";
    sheetname = "effect_cheesebit";
    sheetIsBitmap = false;
    spritewidth = 10;
    spriteheight = 10;
    spriteCenterX = 5;
    spriteCenterY = 5;
    spriteTargetX = 5;
    spriteTargetY = 5;
    sheetWidth = 3;
    sheetHeight = 1;
    gravity = 1;
    normalgravity = 1;
    jumpstart = -14;
    speed = 4;
    foreground = true;
}

// AS: CheeseBitEffect.grabAnimationCycle — the piece named by the subtype.
const AnimationCycle &CheeseBitEffect::grabAnimationCycle()
{
    return cycleFrames(subtype.empty() ? "piece1CycleFrames" : subtype + "CycleFrames");
}

// AS: CheeseBitEffect.endAnimationCycle
void CheeseBitEffect::endAnimationCycle()
{
    gameObj->obf_V->removeEffect(id);
}

// AS: CheeseBitEffect.updateObject — it goes once it lands.
void CheeseBitEffect::updateObject()
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
