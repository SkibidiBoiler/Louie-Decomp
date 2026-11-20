#include "game/papaGame/models/effects/BossEscapeEffect.h"

#include <cmath>

#include <SDL3/SDL.h>

#include "engine/audio.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: BossEscapeEffect.BossEscapeEffect
void BossEscapeEffect::init(double x_, double y_, double id_, const std::string &subtype_, bool followPlayer_,
                            double playerXoffset_, double playerYoffset_)
{
    Effect::init(x_, y_, id_, subtype_, followPlayer_, playerXoffset_, playerYoffset_);
    targetY = gameObj->playerObj->y - 150;
    ydist = targetY - y;
    initialY = y;
    actualY = y;
    offScreenY = gameObj->playerObj->y - 380;
}

// AS: BossEscapeEffect.defineVars
void BossEscapeEffect::defineVars()
{
    effectType = "BossEscapeEffect";
    sheetname = "effect_bossfly";
    sheetIsBitmap = false;
    spritewidth = 104;
    spriteheight = 150;
    spriteCenterX = 52;
    spriteCenterY = 75;
    spriteTargetX = 52;
    spriteTargetY = 75;
    height = 4;
    width = 4;
    collRect = {-4, -4, 8, 8};
    sheetWidth = 3;
    sheetHeight = 1;
    gravity = 1;
    normalgravity = 1;
    jumpstart = -14;
    speed = 4;
    foreground = true;
}

// AS: BossEscapeEffect.grabAnimationCycle
const AnimationCycle &BossEscapeEffect::grabAnimationCycle()
{
    return cycleFrames("animCycleFrames");
}

// AS: BossEscapeEffect.endAnimationCycle — never reached: "anim" loops.
void BossEscapeEffect::endAnimationCycle()
{
    gameObj->obf_V->removeEffect(id);
}

// AS: BossEscapeEffect.updateObject
void BossEscapeEffect::updateObject()
{
    const double tile = gameObj->obf_71->tileWidth;
    if (!copterSound) {
        copterSound = gameObj->obf_7C->playSound("fitzcopter.wav", true, getProximityVolume(), getProximityPan());
    } else {
        copterSound->setTransform(0.3 + getProximityVolume() * 0.7, getProximityPan());
    }
    if (isTweeningToPlayer) {
        const double dist = targetY - y;
        y += dist / 16;
        actualY = y;
        ytile = std::floor(y / tile);
        if (std::abs(dist) <= 4) {
            SDL_Log("(atplayer)");
            isTweeningToPlayer = false;
        }
    } else if (isTweeningOut) {
        const double dist = offScreenY - y;
        y += dist / 16;
        actualY = y;
        ytile = std::floor(y / tile);
        if (std::abs(dist) <= 10) {
            SDL_Log("(out)");
            isOut = true;
            y = offScreenY;
        }
    } else if (!isOut) {
        ++waitTimer;
        if (waitTimer > waitTimerMax) {
            isTweeningOut = true;
        }
        ++waveTimer;
        y = actualY + std::sin(waveTimer / wavePeriod) * waveAmplitude;
        ytile = std::floor(y / tile);
    }
    updateSprite();
}

// AS: BossEscapeEffect.destroy
void BossEscapeEffect::destroy()
{
    if (copterSound) {
        copterSound->stop();
        copterSound.reset();
    }
    Effect::destroy();
}

} // namespace papaGame::models::effects
