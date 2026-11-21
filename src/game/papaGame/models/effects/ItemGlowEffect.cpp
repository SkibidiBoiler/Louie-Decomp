#include "game/papaGame/models/effects/ItemGlowEffect.h"

#include <cmath>

#include "game/as3.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: ItemGlowEffect constructor — the subtype names the effect to follow.
void ItemGlowEffect::init(double x_, double y_, double id_, const std::string &subtype_, bool followPlayer_,
                          double playerXoffset_, double playerYoffset_)
{
    Effect::init(x_, y_, id_, subtype_, followPlayer_, playerXoffset_, playerYoffset_);
    targetEffectID = as3::Number(subtype);
    subtype = "";
    animationFrame = 9;
}

// AS: ItemGlowEffect.defineVars
void ItemGlowEffect::defineVars()
{
    effectType = "ItemGlowEffect";
    sheetname = "item_glow_effect";
    spritewidth = 150;
    spriteheight = 150;
    spriteCenterX = 75;
    spriteCenterY = 75;
    spriteTargetX = 75;
    spriteTargetY = 75;
    sheetWidth = 10;
    sheetHeight = 1;
    foreground = false;
}

// AS: ItemGlowEffect.grabAnimationCycle
const AnimationCycle &ItemGlowEffect::grabAnimationCycle()
{
    if (fadingIn) {
        return fadeInCycleFrames;
    }
    if (fadingOut) {
        return fadeOutCycleFrames;
    }
    return animCycleFrames;
}

// AS: ItemGlowEffect.endAnimationCycle
void ItemGlowEffect::endAnimationCycle()
{
    if (fadingIn) {
        fadingIn = false;
    } else if (fadingOut) {
        gameObj->obf_V->removeEffect(id);
    }
}

// AS: ItemGlowEffect.stopGlow
void ItemGlowEffect::stopGlow()
{
    fadingIn = false;
    fadingOut = true;
}

// AS: ItemGlowEffect.updateObject — on its effect while that exists; off the screen it goes.
void ItemGlowEffect::updateObject()
{
    managers::EffectManager &fx = *gameObj->obf_V;
    const display::GameDisplay &d = *gameObj->obf_71;
    if (d.checkOnScreen(*this)) {
        if (!fadingOut) {
            if (const Effect *target = fx.getEffect(targetEffectID)) {
                x = target->x;
                y = target->y;
                xtile = std::floor(x / d.tileWidth);
                ytile = std::floor(y / d.tileWidth);
            }
        }
        updateSprite();
    } else {
        fx.removeEffect(id);
    }
}

} // namespace papaGame::models::effects
