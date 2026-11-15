#include "game/papaGame/models/effects/DustEffect.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: DustEffect constructor — for "anim" the offsets give the drift: x sign = direction, y sign = up or down.
void DustEffect::init(double x_, double y_, double id_, const std::string &subtype_, bool followPlayer_,
                      double playerXoffset_, double playerYoffset_)
{
    Effect::init(x_, y_, id_, subtype_, followPlayer_, playerXoffset_, playerYoffset_);
    if (subtype == "anim") {
        dirx = playerXoffset_ < 0 ? -1 : (playerXoffset_ > 0 ? 1 : 0);
        if (playerYoffset_ > 0) {
            jump = true;
            jumpspeed = 0;
        } else if (playerYoffset_ < 0) {
            jump = true;
            jumpspeed = jumpstart;
        } else {
            jump = false;
        }
    }
}

// AS: DustEffect.defineVars
void DustEffect::defineVars()
{
    effectType = "DustEffect";
    sheetname = "dust_effect_sheet";
    spritewidth = 20;
    spriteheight = 20;
    spriteCenterX = 10;
    spriteCenterY = 10;
    spriteTargetX = 10;
    spriteTargetY = 10;
    sheetWidth = 7;
    sheetHeight = 1;
    gravity = 1;
    normalgravity = 1;
    jumpstart = -4;
    speed = 2;
    foreground = true;
}

// AS: DustEffect.grabAnimationCycle
const AnimationCycle &DustEffect::grabAnimationCycle()
{
    return cycleFrames("piece1CycleFrames");
}

// AS: DustEffect.endAnimationCycle
void DustEffect::endAnimationCycle()
{
    gameObj->obf_V->removeEffect(id);
}

// AS: DustEffect.updateObject
void DustEffect::updateObject()
{
    if (gameObj->obf_71->checkOnScreen(*this)) {
        if (subtype == "anim") {
            if (jump) {
                jumpChar(false);
            }
            if (dirx != 0) {
                moveChar(dirx, 0, 0, false, false, false);
            }
        }
        updateSprite();
    } else {
        gameObj->obf_V->removeEffect(id);
    }
}

} // namespace papaGame::models::effects
