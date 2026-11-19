#include "game/papaGame/models/effects/DoorFrontEffect.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: DoorFrontEffect.defineVars
void DoorFrontEffect::defineVars()
{
    effectType = "DoorFrontEffect";
    sheetname = "object_doorfront";
    sheetIsBitmap = false;
    spritewidth = 46;
    spriteheight = 136;
    spriteCenterX = 23;
    spriteCenterY = 68;
    spriteTargetX = 16;
    spriteTargetY = 16;
    sheetWidth = 1;
    sheetHeight = 1;
    gravity = 1;
    normalgravity = 1;
    jumpstart = -4;
    speed = 2;
    foreground = true;
}

// AS: DoorFrontEffect.grabAnimationCycle
const AnimationCycle &DoorFrontEffect::grabAnimationCycle()
{
    return cycleFrames("frontCycleFrames");
}

// AS: DoorFrontEffect.updateObject
void DoorFrontEffect::updateObject()
{
    if (gameObj->obf_71->checkOnScreen(*this)) {
        updateSprite();
    }
}

// AS: DoorFrontEffect.checkOnScreen — two columns either side of the screen.
bool DoorFrontEffect::checkOnScreen() const
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 2 && xtile < d.currentXtileRight + 2;
}

} // namespace papaGame::models::effects
