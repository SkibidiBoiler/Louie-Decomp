#include "game/papaGame/models/effects/DizzyEffect.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: DizzyEffect.defineVars
void DizzyEffect::defineVars()
{
    effectType = "DizzyEffect";
    sheetname = "effect_dizzy";
    sheetIsBitmap = false;
    spritewidth = 80;
    spriteheight = 40;
    spriteCenterX = 40;
    spriteCenterY = 20;
    spriteTargetX = 40;
    spriteTargetY = 20;
    sheetWidth = 16;
    sheetHeight = 2;
    gravity = 1;
    normalgravity = 1;
    jumpstart = -4;
    speed = 2;
    foreground = true;
}

// AS: DizzyEffect.grabAnimationCycle
const AnimationCycle &DizzyEffect::grabAnimationCycle()
{
    return cycleFrames(subtype == "faceLeft" ? "faceLeftCycleFrames" : "faceRightCycleFrames");
}

// AS: DizzyEffect.updateObject — following the player, it turns with him.
void DizzyEffect::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    if (d.checkOnScreen(*this)) {
        if (followPlayer) {
            const auto &p = *gameObj->playerObj;
            x = p.x + playerXoffset;
            y = p.y + playerYoffset;
            xtile = std::floor(x / d.tileWidth);
            ytile = std::floor(y / d.tileWidth);
            if (p.facingDir == 1 && cycleName == "faceLeft") {
                subtype = "faceRight";
                cycleName = "faceRight";
                animationFrame = cycleFrames("faceRightCycleFrames").frameAt(cycleFrame);
            } else if (p.facingDir == -1 && cycleName == "faceRight") {
                subtype = "faceLeft";
                cycleName = "faceLeft";
                animationFrame = cycleFrames("faceLeftCycleFrames").frameAt(cycleFrame);
            }
        }
        updateSprite();
    }
}

// AS: DizzyEffect.stopStun
void DizzyEffect::stopStun()
{
    gameObj->obf_V->removeEffect(id);
}

} // namespace papaGame::models::effects
