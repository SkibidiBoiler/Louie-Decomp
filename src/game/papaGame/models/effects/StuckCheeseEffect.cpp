#include "game/papaGame/models/effects/StuckCheeseEffect.h"

#include <cmath>
#include <cstdint>
#include <string>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::effects {

// AS: StuckCheeseEffect.defineVars
void StuckCheeseEffect::defineVars()
{
    effectType = "StuckCheeseEffect";
    sheetname = "effect_stuckcheese";
    sheetIsBitmap = false;
    spritewidth = 72;
    spriteheight = 72;
    spriteCenterX = 36;
    spriteCenterY = 36;
    spriteTargetX = 36;
    spriteTargetY = 40;
    sheetWidth = 9;
    sheetHeight = 1;
    gravity = 1;
    normalgravity = 1;
    jumpstart = -4;
    speed = 2;
    foreground = true;
}

// AS: StuckCheeseEffect.grabAnimationCycle — the lump's size (3 to 1), shaking while the player jumps.
const AnimationCycle &StuckCheeseEffect::grabAnimationCycle()
{
    if (amountOfCheese < 1) {
        amountOfCheese = 1;
    }
    const std::string amount = std::to_string(static_cast<int32_t>(amountOfCheese));
    return cycleFrames((playerJumping ? "jumpCheese" : "cheese") + amount + "CycleFrames");
}

// AS: StuckCheeseEffect.endAnimationCycle — a shake ends with one piece fewer.
void StuckCheeseEffect::endAnimationCycle()
{
    if (cycleName == "jumpCheese3" || cycleName == "jumpCheese2" || cycleName == "jumpCheese1") {
        --amountOfCheese;
        playerJumping = false;
    }
}

// AS: StuckCheeseEffect.updateObject — it follows the player until they land, then stays stuck there.
void StuckCheeseEffect::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    if (!d.checkOnScreen(*this)) {
        return;
    }
    if (followPlayer) {
        const auto &p = *gameObj->playerObj;
        x = p.x + playerXoffset;
        y = p.y + playerYoffset;
        xtile = std::floor(x / d.tileWidth);
        ytile = std::floor(y / d.tileWidth);
        if (!p.jump && !p.onMovingTile) {
            followPlayer = false;
        }
    }
    updateSprite();
    if (amountOfCheese <= 0) {
        gameObj->playerObj->removeCheese();
    }
}

// AS: StuckCheeseEffect.playerJumped — crumbs fly off.
void StuckCheeseEffect::playerJumped()
{
    playerJumping = true;
    managers::EffectManager &fx = *gameObj->obf_V;
    fx.addEffect(x, y, "CheeseBitEffect", "piece1");
    fx.addEffect(x - 3, y - 1, "CheeseBitEffect", "piece2");
    fx.addEffect(x + 3, y - 1, "CheeseBitEffect", "piece2");
    fx.addEffect(x, y, "CheeseBitEffect", "piece3");
    fx.addEffect(x, y, "CheeseBitEffect", "piece3");
}

// AS: StuckCheeseEffect.refillCheese
void StuckCheeseEffect::refillCheese()
{
    amountOfCheese = 3;
}

// AS: StuckCheeseEffect.checkOnScreen — only the columns count.
bool StuckCheeseEffect::checkOnScreen() const
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 3 && xtile < d.currentXtileRight + 3;
}

} // namespace papaGame::models::effects
