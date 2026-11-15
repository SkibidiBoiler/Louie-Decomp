#include "game/papaGame/models/effects/PlayerCheckpointEffect.h"

#include <cmath>

#include "game/obf_AF/obf_1n.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: PlayerCheckpointEffect constructor — the six stars start 60 degrees apart.
void PlayerCheckpointEffect::init(double x_, double y_, double id_, const std::string &subtype_, bool followPlayer_,
                                  double playerXoffset_, double playerYoffset_)
{
    Effect::init(x_, y_, id_, subtype_, followPlayer_, playerXoffset_, playerYoffset_);
    static const char *const stars[] = {"star2", "star3", "star4", "star5", "star6"};
    for (int i = 0; i < 5; ++i) {
        if (subtype_ == stars[i]) {
            blimpAngle = obf_AF::obf_1n::degreesToRads(-60.0 * (i + 1));
            rotationTimer = blimpAngle / (walkspeed / radius);
        }
    }
    flightCircleX = x;
    flightCircleY = y;
}

// AS: PlayerCheckpointEffect.defineVars
void PlayerCheckpointEffect::defineVars()
{
    effectType = "PlayerCheckpointEffect";
    sheetname = "effect_playerstun";
    sheetIsBitmap = false;
    spritewidth = 16;
    spriteheight = 16;
    spriteCenterX = 8;
    spriteCenterY = 8;
    spriteTargetX = 8;
    spriteTargetY = 8;
    sheetWidth = 5;
    sheetHeight = 1;
    gravity = 1;
    normalgravity = 1;
    jumpstart = -4;
    speed = 12;
    walkspeed = 12;
    foreground = true;
}

// AS: PlayerCheckpointEffect.grabAnimationCycle
const AnimationCycle &PlayerCheckpointEffect::grabAnimationCycle()
{
    return cycleFrames(subtype.empty() ? "star1CycleFrames" : subtype + "CycleFrames");
}

// AS: PlayerCheckpointEffect.updateObject — circles the player, then keeps its speed and accelerates away.
void PlayerCheckpointEffect::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    if (!d.checkOnScreen(*this)) {
        gameObj->obf_V->removeEffect(id);
        return;
    }
    ++rotationTimer;
    if (followPlayer) {
        const double lastDX = x - flightCircleX, lastDY = y - flightCircleY;
        flightCircleX = gameObj->playerObj->x + playerXoffset;
        flightCircleY = gameObj->playerObj->y + playerYoffset;
        const double step = walkspeed * rotationDirection / radius;
        x = std::round(flightCircleX + radius * std::cos(step * rotationTimer));
        y = std::round(flightCircleY + radius * std::sin(step * rotationTimer));
        xtile = std::floor(x / d.tileWidth);
        ytile = std::floor(y / d.tileWidth);
        xspeed = x - flightCircleX - lastDX;
        yspeed = y - flightCircleY - lastDY;
    } else {
        xspeed *= 1.05;
        yspeed *= 1.05;
        x += xspeed;
        y += yspeed;
        xtile = std::floor(x / d.tileWidth);
        ytile = std::floor(y / d.tileWidth);
    }
    ++spinTimer;
    if (spinTimer >= spinTimerMax && followPlayer) {
        stopSpinning();
    }
    updateSprite();
}

// AS: PlayerCheckpointEffect.stopSpinning
void PlayerCheckpointEffect::stopSpinning()
{
    followPlayer = false;
    rotationTimer += 10;
}

} // namespace papaGame::models::effects
