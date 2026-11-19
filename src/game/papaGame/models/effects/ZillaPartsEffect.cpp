#include "game/papaGame/models/effects/ZillaPartsEffect.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: ZillaPartsEffect constructor — thrown up and to a side.
void ZillaPartsEffect::init(double x_, double y_, double id_, const std::string &subtype_, bool followPlayer_,
                      double playerXoffset_, double playerYoffset_)
{
    Effect::init(x_, y_, id_, subtype_, followPlayer_, playerXoffset_, playerYoffset_);
    jump = true;
    dirx = 1;
    if (gameObj->playerObj->x > x) {
        dirx = -1;
    }
    speed = 2 + std::floor(as3::random() * 4);
    animationFrame = grabAnimationCycle().frameAt(0);
}

// AS: ZillaPartsEffect.defineVars
void ZillaPartsEffect::defineVars()
{
    effectType = "ZillaPartsEffect";
    sheetname = "effect_zillaparts";
    sheetIsBitmap = false;
    spritewidth = 58;
    spriteheight = 36;
    spriteCenterX = 29;
    spriteCenterY = 18;
    spriteTargetX = 29;
    spriteTargetY = 18;
    sheetWidth = 4;
    sheetHeight = 1;
    gravity = 1;
    normalgravity = 1;
    jumpstart = -10;
    speed = 4;
    foreground = true;
}

// AS: ZillaPartsEffect.grabAnimationCycle — the piece named by the subtype.
const AnimationCycle &ZillaPartsEffect::grabAnimationCycle()
{
    return cycleFrames(subtype.empty() ? "piece1CycleFrames" : subtype + "CycleFrames");
}

// AS: ZillaPartsEffect.endAnimationCycle
void ZillaPartsEffect::endAnimationCycle()
{
    gameObj->obf_V->removeEffect(id);
}

// AS: ZillaPartsEffect.updateObject
void ZillaPartsEffect::updateObject()
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
