#include "game/papaGame/models/effects/CageCustomerEffect.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: CageCustomerEffect.CageCustomerEffect — the cage passes its facing in playerXoffset.
void CageCustomerEffect::init(double x_, double y_, double id_, const std::string &subtype_, bool followPlayer_,
                              double playerXoffset_, double playerYoffset_)
{
    Effect::init(x_, y_, id_, subtype_, followPlayer_, playerXoffset_, playerYoffset_);
    facingDir = playerXoffset_;
    playerXoffset = 0;
    flipBlitOffset = facingDir == 1 ? 0 : spriteheight;
    dirx = 0;
}

// AS: CageCustomerEffect.defineVars — the sprite size comes from the first caged sheet (16 x 2 cells).
void CageCustomerEffect::defineVars()
{
    sheetWidth = 16;
    sheetHeight = 2;
    effectType = "CageCustomerEffect";
    sheetname = "blanksprite_sheet";
    sheetIsBitmap = true;
    const engine::Sheet &caged1 = gameObj->bitmapManager->caged1Sheet;
    spritewidth = caged1.width / sheetWidth;
    spriteheight = caged1.height / sheetHeight;
    spriteCenterX = std::round(spritewidth / 2);
    spriteCenterY = std::round(spriteheight / 2);
    spriteTargetX = std::round(spritewidth / 2);
    spriteTargetY = spriteheight - 25;
    gravity = 2;
    normalgravity = 2;
    jumpstart = -22;
    speed = 8;
    foreground = true;
    dirx = 0;
}

// AS: CageCustomerEffect.grabAnimationCycle
const AnimationCycle &CageCustomerEffect::grabAnimationCycle()
{
    if (isGone) {
        return goneCycleFrames;
    }
    if (isOpen) {
        if (!hasStoodUp) {
            return openCycleFrames;
        }
        if (jump && jumpspeed < 0) {
            return jumpCycleFrames;
        }
        if (jump) {
            return fallCycleFrames;
        }
        return standCycleFrames;
    }
    return closedCycleFrames;
}

// AS: CageCustomerEffect.endAnimationCycle
void CageCustomerEffect::endAnimationCycle()
{
    if (cycleName == "open") {
        hasStoodUp = true;
    }
}

// AS: CageCustomerEffect.updateObject — QUIRK: the portal check at the end does nothing (an empty if).
void CageCustomerEffect::updateObject()
{
    if (gameObj->obf_71->checkOnScreen(*this)) {
        updateSprite();
        if (jump && portalX == -1) {
            jumpChar();
        }
        if (portalX != -1 && portalY != -1) {
            const double distx = portalX - x;
            const double disty = portalY - y;
            x += distx / 3;
            y += disty / 3;
            jump = true;
            jumpspeed = -1;
        }
    }
}

// AS: CageCustomerEffect.checkOnScreen
bool CageCustomerEffect::checkOnScreen() const
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 3 && xtile < d.currentXtileRight + 3;
}

// AS: CageCustomerEffect.getSheet — the sheet of its cage (subtype "1".."3").
const engine::Sheet *CageCustomerEffect::getSheet() const
{
    const managers::BitmapManager &bm = *gameObj->bitmapManager;
    const engine::Sheet *sheet = &bm.caged1Sheet;
    if (subtype == "3") {
        sheet = &bm.caged3Sheet;
    } else if (subtype == "2") {
        sheet = &bm.caged2Sheet;
    }
    return sheet->texture ? sheet : nullptr;
}

} // namespace papaGame::models::effects
