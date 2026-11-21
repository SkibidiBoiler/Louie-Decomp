#include "game/papaGame/models/effects/PlayerStunSparkleEffect.h"

#include <cmath>

#include "game/as3.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: PlayerStunSparkleEffect constructor — the player offsets passed in are the head's centre.
void PlayerStunSparkleEffect::init(double x_, double y_, double id_, const std::string &subtype_, bool followPlayer_,
                                   double playerXoffset_, double playerYoffset_)
{
    Effect::init(x_, y_, id_, subtype_, followPlayer_, playerXoffset_, playerYoffset_);
    headCenterX = playerXoffset_;
    headCenterY = playerYoffset_;
    changePlacement();
}

// AS: PlayerStunSparkleEffect.defineVars
void PlayerStunSparkleEffect::defineVars()
{
    effectType = "PlayerStunSparkleEffect";
    sheetname = "effect_playerstunsparkle";
    sheetIsBitmap = false;
    spritewidth = 16;
    spriteheight = 16;
    spriteCenterX = 8;
    spriteCenterY = 8;
    spriteTargetX = 8;
    spriteTargetY = 8;
    sheetWidth = 8;
    sheetHeight = 1;
    gravity = 1;
    normalgravity = 1;
    jumpstart = -4;
    speed = 12;
    walkspeed = 12;
    foreground = true;
}

// AS: PlayerStunSparkleEffect.changePlacement — a new wait and spot within 40 px of the head, pushed out of the
// middle band by 20 px.
void PlayerStunSparkleEffect::changePlacement()
{
    waitTimer = 0;
    waitTimerMax = std::floor(as3::random() * 15);
    playerXoffset = -20 + as3::round(as3::random() * 40);
    playerYoffset = -20 + as3::round(as3::random() * 40);
    if (playerYoffset > -20 && playerYoffset < 20) {
        if (playerXoffset < 0) {
            playerXoffset -= 20;
        } else {
            playerXoffset += 20;
        }
    }
    playerXoffset += headCenterX;
    playerYoffset += headCenterY;
}

// AS: PlayerStunSparkleEffect.grabAnimationCycle
const AnimationCycle &PlayerStunSparkleEffect::grabAnimationCycle()
{
    if (waitTimer >= waitTimerMax) {
        return animCycleFrames;
    }
    return waitCycleFrames;
}

// AS: PlayerStunSparkleEffect.endAnimationCycle
void PlayerStunSparkleEffect::endAnimationCycle()
{
    if (isEnding) {
        gameObj->obf_V->removeEffect(id);
    } else {
        changePlacement();
    }
}

// AS: PlayerStunSparkleEffect.updateObject
void PlayerStunSparkleEffect::updateObject()
{
    managers::EffectManager &fx = *gameObj->obf_V;
    const display::GameDisplay &d = *gameObj->obf_71;
    if (d.checkOnScreen(*this)) {
        ++waitTimer;
        if (followPlayer) {
            x = gameObj->playerObj->x + playerXoffset;
            y = gameObj->playerObj->y + playerYoffset;
            xtile = std::floor(x / d.tileWidth);
            ytile = std::floor(y / d.tileWidth);
        }
        updateSprite();
    } else {
        fx.removeEffect(id);
    }
}

// AS: PlayerStunSparkleEffect.stopStun
void PlayerStunSparkleEffect::stopStun()
{
    isEnding = true;
}

} // namespace papaGame::models::effects
