#include "game/papaGame/models/effects/BeakerGlassEffect.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: BeakerGlassEffect constructor — thrown up and to a side.
void BeakerGlassEffect::init(double x_, double y_, double id_, const std::string &subtype_, bool followPlayer_,
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

// AS: BeakerGlassEffect.defineVars
void BeakerGlassEffect::defineVars()
{
    effectType = "BeakerGlassEffect";
    sheetname = "effect_beakerglass";
    sheetIsBitmap = false;
    spritewidth = 16;
    spriteheight = 16;
    spriteCenterX = 8;
    spriteCenterY = 8;
    spriteTargetX = 8;
    spriteTargetY = 8;
    sheetWidth = 4;
    sheetHeight = 1;
    gravity = 1;
    normalgravity = 1;
    jumpstart = -14;
    speed = 4;
    foreground = true;
}

// AS: BeakerGlassEffect.grabAnimationCycle — the piece named by the subtype.
const AnimationCycle &BeakerGlassEffect::grabAnimationCycle()
{
    return cycleFrames(subtype.empty() ? "piece1CycleFrames" : subtype + "CycleFrames");
}

// AS: BeakerGlassEffect.endAnimationCycle
void BeakerGlassEffect::endAnimationCycle()
{
    gameObj->obf_V->removeEffect(id);
}

// AS: BeakerGlassEffect.updateObject
void BeakerGlassEffect::updateObject()
{
    if (gameObj->obf_71->checkOnScreen(*this)) {
        jumpChar(false);
        moveChar(dirx, 0, 0, false, false, false);
    } else {
        gameObj->obf_V->removeEffect(id);
    }
}

} // namespace papaGame::models::effects
