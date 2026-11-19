#include "game/papaGame/models/effects/StarEffect.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: StarEffect constructor — thrown up and to a side.
void StarEffect::init(double x_, double y_, double id_, const std::string &subtype_, bool followPlayer_,
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

// AS: StarEffect.defineVars
void StarEffect::defineVars()
{
    effectType = "StarEffect";
    sheetname = "effect_playerstun";
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

// AS: StarEffect.grabAnimationCycle — the piece named by the subtype.
const AnimationCycle &StarEffect::grabAnimationCycle()
{
    return cycleFrames(subtype.empty() ? "piece1CycleFrames" : subtype + "CycleFrames");
}

// AS: StarEffect.endAnimationCycle
void StarEffect::endAnimationCycle()
{
    gameObj->obf_V->removeEffect(id);
}

// AS: StarEffect.updateObject
void StarEffect::updateObject()
{
    if (gameObj->obf_71->checkOnScreen(*this)) {
        if (subtype != "static") {
            jumpChar(false);
            moveChar(dirx, 0, 0, false, false, false);
        }
        updateSprite();
    } else {
        gameObj->obf_V->removeEffect(id);
    }
}

} // namespace papaGame::models::effects
