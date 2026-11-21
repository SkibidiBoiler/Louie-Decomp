#include "game/papaGame/models/effects/BossPropellerEffect.h"

#include <cmath>

#include <SDL3/SDL_log.h>

#include "game/as3.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: BossPropellerEffect constructor — the subtype carries the boss's id.
void BossPropellerEffect::init(double x_, double y_, double id_, const std::string &subtype_, bool followPlayer_,
                               double playerXoffset_, double playerYoffset_)
{
    Effect::init(x_, y_, id_, subtype_, followPlayer_, playerXoffset_, playerYoffset_);
    bossID = as3::Number(subtype);
    subtype = "";
}

// AS: BossPropellerEffect.defineVars
void BossPropellerEffect::defineVars()
{
    effectType = "BossPropellerEffect";
    sheetname = "effect_bossspinner";
    sheetIsBitmap = false;
    spritewidth = 100;
    spriteheight = 30;
    spriteCenterX = 50;
    spriteCenterY = 15;
    spriteTargetX = 50;
    spriteTargetY = 15;
    sheetWidth = 18;
    sheetHeight = 1;
    gravity = 1;
    normalgravity = 1;
    jumpstart = -4;
    speed = 2;
    foreground = false;
}

// AS: BossPropellerEffect.grabAnimationCycle
const AnimationCycle &BossPropellerEffect::grabAnimationCycle()
{
    return cycleFrames("spinCycleFrames");
}

// AS: BossPropellerEffect.updateObject — on the boss plus its offsets (the original logs an error when the boss is
// gone).
void BossPropellerEffect::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    if (const Enemy *boss = gameObj->obf_E->enemyByID(bossID)) {
        x = boss->x + playerXoffset;
        y = boss->y + playerYoffset;
        xtile = std::floor(x / d.tileWidth);
        ytile = std::floor(y / d.tileWidth);
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error with propeller.....");
    }
    updateSprite();
}

} // namespace papaGame::models::effects
