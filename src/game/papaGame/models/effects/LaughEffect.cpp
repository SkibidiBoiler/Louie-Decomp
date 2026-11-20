#include "game/papaGame/models/effects/LaughEffect.h"

#include <cmath>

#include "game/obf_AF/obf_AJ.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: LaughEffect.LaughEffect (its §§push/§§pop remains read as targetY = y - ydist)
void LaughEffect::init(double x_, double y_, double id_, const std::string &subtype_, bool followPlayer_,
                       double playerXoffset_, double playerYoffset_)
{
    Effect::init(x_, y_, id_, subtype_, followPlayer_, playerXoffset_, playerYoffset_);
    targetY = y - ydist;
    initialY = y;
}

// AS: LaughEffect.defineVars
void LaughEffect::defineVars()
{
    effectType = "LaughEffect";
    sheetname = "effect_laugh";
    sheetIsBitmap = false;
    spritewidth = 36;
    spriteheight = 36;
    spriteCenterX = 18;
    spriteCenterY = 18;
    spriteTargetX = 18;
    spriteTargetY = 18;
    height = 4;
    width = 4;
    collRect = {-4, -4, 8, 8};
    sheetWidth = 25;
    sheetHeight = 1;
    gravity = 1;
    normalgravity = 1;
    jumpstart = -14;
    speed = 4;
    foreground = true;
}

// AS: LaughEffect.grabAnimationCycle — `this[subtype + "CycleFrames"]`.
const AnimationCycle &LaughEffect::grabAnimationCycle()
{
    return cycleFrames(subtype + "CycleFrames");
}

// AS: LaughEffect.endAnimationCycle
void LaughEffect::endAnimationCycle()
{
    gameObj->obf_V->removeEffect(id);
}

// AS: LaughEffect.updateObject
void LaughEffect::updateObject()
{
    y = obf_AF::obf_AJ::obf_5w(animTimer, initialY, 0 - ydist, timeToRise);
    ++animTimer;
    ytile = std::floor(y / gameObj->obf_71->tileWidth);
    updateSprite();
}

} // namespace papaGame::models::effects
