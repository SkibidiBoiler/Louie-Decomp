#include "game/papaGame/models/effects/FoodiniGlideEffect.h"

#include <algorithm>
#include <cmath>

#include "game/as3.h"
#include "game/obf_9G/obf_4R.h"
#include "game/obf_AF/obf_AJ.h"
#include "game/papaGame/models/effects/FoodiniBalloonEffect.h"

namespace papaGame::models::effects {

// AS: FoodiniGlideEffect constructor — a random easing speed, rise and colour.
void FoodiniGlideEffect::init(double x_, double y_, double id_, const std::string &subtype_, bool followPlayer_,
                              double playerXoffset_, double playerYoffset_)
{
    Effect::init(x_, y_, id_, subtype_, followPlayer_, playerXoffset_, playerYoffset_);
    tweenSpeed = 2 + std::floor(as3::random() * 3);
    riseDecay = 2 + std::floor(as3::random() * 3);
    const double colour = std::floor(as3::random() * 4);
    animCycleFrames.loopFrame = colour;
    animCycleFrames.firstFrame = colour;
    animCycleFrames.lastFrame = colour;
    animationFrame = colour;
    pathY = y;
}

// AS: FoodiniGlideEffect.defineVars
void FoodiniGlideEffect::defineVars()
{
    effectType = "FoodiniGlideEffect";
    sheetname = "effect_foodiniglide";
    sheetIsBitmap = false;
    spritewidth = 24;
    spriteheight = 32;
    spriteCenterX = 12;
    spriteCenterY = 16;
    spriteTargetX = 12;
    spriteTargetY = 16;
    sheetWidth = 19;
    sheetHeight = 1;
    foreground = false;
}

// AS: FoodiniGlideEffect.grabAnimationCycle
const AnimationCycle &FoodiniGlideEffect::grabAnimationCycle()
{
    return animCycleFrames;
}

// AS: FoodiniGlideEffect.updateObject — let go, it rises faster (one more every riseDecay frames) until it is off
// the screen.
void FoodiniGlideEffect::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    updateSprite();
    if (isAttached) {
        floatTethered();
    } else {
        if (std::fmod(gameObj->obf_8o->gameplayTimer, riseDecay) == 0 && speed < 12) {
            ++speed;
        }
        y -= speed;
        ytile = std::floor(y / d.tileWidth);
        if (!checkOnScreen()) {
            gameObj->obf_V->removeEffect(id);
        }
    }
}

// AS: FoodiniGlideEffect.checkOnScreen
bool FoodiniGlideEffect::checkOnScreen() const
{
    const display::GameDisplay &d = *gameObj->obf_71;
    if (isAttached) {
        return true;
    }
    return xtile > d.currentXtileLeft - 2 && xtile < d.currentXtileRight + 2 && ytile > d.currentYtileUp - 2 &&
           ytile < d.currentYtileDown;
}

// AS: FoodiniGlideEffect.floatTethered — eases 1/tweenSpeed of the way to its spot above Foodini a frame and bobs;
// reeled in, it heads for his hand and goes when within 24 px.
void FoodiniGlideEffect::floatTethered()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    const double tw = d.tileWidth;
    const characters::CustomerChar &p = *gameObj->playerObj;
    double targetX = p.x + playerXoffset;
    double targetY = p.y + playerAnchorOffset + playerYoffset;
    if (isReelingIn) {
        targetX = p.x;
        targetY = p.y - 32;
        tweenSpeed = 2;
    }
    const double dx = targetX - x;
    const double dy = targetY - pathY;
    x += dx / tweenSpeed;
    pathY += dy / tweenSpeed;
    xtile = std::floor(x / tw);
    ytile = std::floor(y / tw);
    double t = waveTimer;
    const double quarter = waveTimerMax / 4;
    if (waveTimer < quarter) {
        y = pathY + obf_AF::obf_AJ::obf_5w(t, 0, waveTimerOffset * -1, quarter);
    } else if (waveTimer < waveTimerMax / 2) {
        t = waveTimer - quarter;
        y = pathY + obf_AF::obf_AJ::obf_7d(t, waveTimerOffset * -1, waveTimerOffset, quarter);
    } else if (waveTimer < quarter * 3) {
        t = waveTimer - waveTimerMax / 2;
        y = pathY + obf_AF::obf_AJ::obf_5w(t, 0, waveTimerOffset, quarter);
    } else if (waveTimer <= waveTimerMax) {
        t = waveTimer - quarter * 3;
        y = pathY + obf_AF::obf_AJ::obf_7d(t, waveTimerOffset, waveTimerOffset * -1, quarter);
        if (waveTimer == waveTimerMax) {
            waveTimer = -1;
        }
    }
    ++waveTimer;
    const auto &tiles = gameObj->obf_AQ->currentScreenData->tileArray;
    const double maxX = (tiles.empty() ? 0 : static_cast<double>(tiles[0].size())) * tw;
    const double maxY = static_cast<double>(tiles.size()) * tw;
    x = std::clamp(x, 0.0, std::max(0.0, maxX));
    y = std::clamp(y, 0.0, std::max(0.0, maxY));
    pathX = x;
    xtile = std::floor(x / tw);
    ytile = std::floor(y / tw);
    if (isReelingIn && std::abs(dx) <= 24 && std::abs(dy) <= 24) {
        gameObj->obf_V->removeEffect(id);
    }
}

// AS: FoodiniGlideEffect.getSheet — the string is drawn first.
const engine::Sheet *FoodiniGlideEffect::getSheet() const
{
    redrawLine();
    return Effect::getSheet();
}

// AS: FoodiniGlideEffect.redrawLine — to a point 10 px behind Foodini's hand (not while reeled in).
void FoodiniGlideEffect::redrawLine() const
{
    if (!isAttached || !isVisible || isReelingIn) {
        return;
    }
    const display::GameDisplay &d = *gameObj->obf_71;
    const characters::CustomerChar &p = *gameObj->playerObj;
    const double fromX = d.drawnX(*this) - spriteTargetX + stringCoordX - d.currentXcoord;
    const double fromY = d.drawnY(*this) - spriteTargetY + stringCoordY - d.currentYcoord;
    const double toX = d.drawnX(p) - 10 * p.facingDir - d.currentXcoord;
    const double toY = d.drawnY(p) + playerAnchorOffset - d.currentYcoord;
    drawBalloonString(d, fromX, fromY, toX, toY, maxDist);
}

// AS: FoodiniGlideEffect.floatAway — let go 40 frames after the last balloons floated away (a third of them after
// 10); sooner, it is reeled back in.
void FoodiniGlideEffect::floatAway()
{
    const double since = gameObj->obf_8o->gameplayTimer - gameObj->playerObj->balloonsLastFloatedAway;
    if (since > 40) {
        speed = 0;
        isAttached = false;
    } else if (std::fmod(id, 3) == 0 && since > 10) {
        speed = 0;
        isAttached = false;
    } else {
        isReelingIn = true;
    }
}

} // namespace papaGame::models::effects
