#include "game/papaGame/models/effects/WarpCoinEffect.h"

#include <cmath>

#include "game/as3.h"
#include "game/obf_9G/obf_4R.h"
#include "game/papaGame/models/effects/ItemGlowEffect.h"

namespace papaGame::models::effects {

// AS: WarpCoinEffect constructor — it rises to 80 px above where it starts, with a glow. (Translated from the
// P-code-like §§push remains: targetY = playerYoffset - targetY.)
void WarpCoinEffect::init(double x_, double y_, double id_, const std::string &subtype_, bool followPlayer_,
                          double playerXoffset_, double playerYoffset_)
{
    Effect::init(x_, y_, id_, subtype_, followPlayer_, playerXoffset_, playerYoffset_);
    targetY = playerYoffset - targetY;
    glowEffectID = gameObj->obf_V->addEffect(x, y, "ItemGlowEffect", as3::String(id));
}

// AS: WarpCoinEffect.defineVars
void WarpCoinEffect::defineVars()
{
    effectType = "WarpCoinEffect";
    sheetname = "effect_warpcoin";
    sheetIsBitmap = false;
    spritewidth = 44;
    spriteheight = 44;
    spriteCenterX = 22;
    spriteCenterY = 22;
    spriteTargetX = 22;
    spriteTargetY = 22;
    height = 4;
    width = 4;
    collRect = {-4, -4, 8, 8};
    sheetWidth = 17;
    sheetHeight = 1;
    gravity = 1;
    normalgravity = 1;
    jumpstart = -14;
    speed = 4;
    foreground = true;
}

// AS: WarpCoinEffect.grabAnimationCycle
const AnimationCycle &WarpCoinEffect::grabAnimationCycle()
{
    return animCycleFrames;
}

// AS: WarpCoinEffect.endAnimationCycle
void WarpCoinEffect::endAnimationCycle()
{
    gameObj->obf_V->removeEffect(id);
}

// AS: WarpCoinEffect.updateObject — its offset eases an eighth of the way to targetY a frame.
void WarpCoinEffect::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    playerYoffset += (targetY - playerYoffset) / 8;
    if (followPlayer) {
        x = gameObj->playerObj->x + playerXoffset;
        y = gameObj->playerObj->y + playerYoffset;
        xtile = std::floor(x / d.tileWidth);
        ytile = std::floor(y / d.tileWidth);
    }
    updateSprite();
}

// AS: WarpCoinEffect.destroy — its glow fades out.
void WarpCoinEffect::destroy()
{
    if (glowEffectID > -1) {
        Effect *glow = gameObj->obf_V->getEffect(glowEffectID);
        if (glow != nullptr && glow->effectType == "ItemGlowEffect") {
            static_cast<ItemGlowEffect *>(glow)->stopGlow();
        }
        glowEffectID = -1;
    }
    Effect::destroy();
}

} // namespace papaGame::models::effects
