#include "game/papaGame/models/effects/BlueberryBitEffect.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: BlueberryBitEffect constructor — thrown up and to a side.
void BlueberryBitEffect::init(double x_, double y_, double id_, const std::string &subtype_, bool followPlayer_,
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

// AS: BlueberryBitEffect.defineVars
void BlueberryBitEffect::defineVars()
{
    effectType = "BlueberryBitEffect";
    sheetname = "effect_blueberrybit";
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

// AS: BlueberryBitEffect.grabAnimationCycle — the piece named by the subtype.
const AnimationCycle &BlueberryBitEffect::grabAnimationCycle()
{
    return cycleFrames(subtype.empty() ? "piece1CycleFrames" : subtype + "CycleFrames");
}

// AS: BlueberryBitEffect.endAnimationCycle
void BlueberryBitEffect::endAnimationCycle()
{
    gameObj->obf_V->removeEffect(id);
}

// AS: BlueberryBitEffect.updateObject
void BlueberryBitEffect::updateObject()
{
    if (gameObj->obf_71->checkOnScreen(*this)) {
        jumpChar(false);
        moveChar(dirx, 0, 0, false, false, false);
    } else {
        gameObj->obf_V->removeEffect(id);
    }
}

} // namespace papaGame::models::effects
