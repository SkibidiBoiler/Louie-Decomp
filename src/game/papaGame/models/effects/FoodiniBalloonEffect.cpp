#include "game/papaGame/models/effects/FoodiniBalloonEffect.h"

#include <algorithm>
#include <cmath>
#include <numbers>

#include "game/as3.h"
#include "game/obf_9G/obf_4R.h"
#include "game/obf_AF/obf_AJ.h"

namespace papaGame::models::effects {

void drawBalloonString(const display::GameDisplay &d, double fromX, double fromY, double toX, double toY,
                       double maxDist)
{
    const double dx = toX - fromX;
    const double dy = toY - fromY;
    if (std::sqrt(dx * dx + dy * dy) > maxDist * 0.7) {
        d.drawScreenLine(fromX, fromY, toX, toY, 1, 0);
        return;
    }
    // curveTo(control = (toX - dx / 2, toY), anchor = (toX, toY)) from (fromX, fromY)
    const double cx = toX - dx / 2, cy = toY;
    double px = fromX, py = fromY;
    constexpr int pieces = 8;
    for (int i = 1; i <= pieces; ++i) {
        const double t = static_cast<double>(i) / pieces, u = 1 - t;
        const double qx = u * u * fromX + 2 * u * t * cx + t * t * toX;
        const double qy = u * u * fromY + 2 * u * t * cy + t * t * toY;
        d.drawScreenLine(px, py, qx, qy, 1, 0);
        px = qx;
        py = qy;
    }
}

// AS: FoodiniBalloonEffect constructor — the "glide" balloons get a random offset and lift.
void FoodiniBalloonEffect::init(double x_, double y_, double id_, const std::string &subtype_, bool followPlayer_,
                                double playerXoffset_, double playerYoffset_)
{
    Effect::init(x_, y_, id_, subtype_, followPlayer_, playerXoffset_, playerYoffset_);
    if (subtype == "glide") {
        playerXoffset = 4 - std::floor(as3::random() * 8);
        playerYoffset = 4 - std::floor(as3::random() * 8);
        liftOffset = std::floor(as3::random() * 3);
        resetBalloon();
    }
}

// AS: FoodiniBalloonEffect.defineVars
void FoodiniBalloonEffect::defineVars()
{
    effectType = "FoodiniBalloonEffect";
    sheetname = "effect_foodiniballoon";
    sheetIsBitmap = false;
    spritewidth = 54;
    spriteheight = 40;
    spriteCenterX = 27;
    spriteCenterY = 20;
    spriteTargetX = 27;
    spriteTargetY = 20;
    sheetWidth = 19;
    sheetHeight = 1;
    foreground = false;
}

// AS: FoodiniBalloonEffect.grabAnimationCycle
const AnimationCycle &FoodiniBalloonEffect::grabAnimationCycle()
{
    return animCycleFrames;
}

// AS: FoodiniBalloonEffect.updateObject — tethered, it follows Foodini (and floats away when he is gone); let go,
// it rises faster and faster until it leaves the screen.
void FoodiniBalloonEffect::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    updateSprite();
    if (isAttached) {
        floatTethered();
        if (gameObj->playerObj->isGone) {
            floatAway();
        }
    } else {
        if (std::fmod(gameObj->obf_8o->gameplayTimer, 2) == 0 && speed < 12) {
            ++speed;
        }
        y -= speed;
        ytile = std::floor(y / d.tileWidth);
        if (!checkOnScreen()) {
            gameObj->obf_V->removeEffect(id);
        }
    }
}

// AS: FoodiniBalloonEffect.checkOnScreen — always while tethered.
bool FoodiniBalloonEffect::checkOnScreen() const
{
    const display::GameDisplay &d = *gameObj->obf_71;
    if (isAttached) {
        return true;
    }
    return xtile > d.currentXtileLeft - 2 && xtile < d.currentXtileRight + 2 && ytile > d.currentYtileUp - 2 &&
           ytile < d.currentYtileDown;
}

// AS: FoodiniBalloonEffect.floatTethered — it rises (faster when Foodini stands, much faster while he glides) up to
// the string's length above his hand, is pulled along within 60 px, and bobs 4 px on eased quarter waves.
void FoodiniBalloonEffect::floatTethered()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    const double tw = d.tileWidth;
    const characters::CustomerChar &p = *gameObj->playerObj;
    const double anchorX = p.x + playerXoffset;
    const double anchorY = p.y + playerAnchorOffset + playerYoffset;
    if (pathY > anchorY - maxDist) {
        pathY -= p.isWalking ? 4 : 6;
        pathY -= liftOffset;
    }
    if (p.isGliding) {
        pathY -= 20;
    }
    const double dx = anchorX - x;
    const double dy = anchorY - pathY;
    const double angle = std::atan2(dy, dx);
    if (std::sqrt(dx * dx + dy * dy) > maxDist) {
        x = anchorX - std::floor(maxDist * std::cos(angle));
        pathY = anchorY - std::floor(maxDist * std::sin(angle));
        y = pathY;
    }
    updateAnimationFrame(angle * (180 / std::numbers::pi));
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
}

// AS: FoodiniBalloonEffect.getSheet — the string is drawn first.
const engine::Sheet *FoodiniBalloonEffect::getSheet() const
{
    redrawLine();
    return Effect::getSheet();
}

// AS: FoodiniBalloonEffect.redrawLine — a 1 px black string from the knot (per frame) to Foodini's hand.
void FoodiniBalloonEffect::redrawLine() const
{
    if (!isAttached || !isVisible) {
        return;
    }
    const display::GameDisplay &d = *gameObj->obf_71;
    const characters::CustomerChar &p = *gameObj->playerObj;
    const auto frame = static_cast<size_t>(animationFrame);
    if (frame >= stringCoordsX.size()) {
        return;
    }
    const double fromX = d.drawnX(*this) - spriteTargetX + stringCoordsX[frame] - d.currentXcoord;
    const double fromY = d.drawnY(*this) - spriteTargetY + stringCoordsY[frame] - d.currentYcoord;
    const double toX = d.drawnX(p) - d.currentXcoord;
    const double toY = d.drawnY(p) + playerAnchorOffset - d.currentYcoord;
    drawBalloonString(d, fromX, fromY, toX, toY, maxDist);
}

// AS: FoodiniBalloonEffect.updateAnimationFrame — frame 0 from -90 to 5 degrees, then one frame per 10 degrees up to
// 18; hidden, frame 19 (past the sheet: nothing is drawn).
void FoodiniBalloonEffect::updateAnimationFrame(double degrees)
{
    double frame = 0;
    if (!isVisible) {
        frame = 19;
    } else if (degrees >= -90 && degrees < 5) {
        frame = 0;
    } else if (degrees >= 5 && degrees < 175) {
        frame = std::floor((degrees - 5) / 10) + 1; // the original's chain of 10-degree ranges
    } else {
        frame = 18;
    }
    animCycleFrames.loopFrame = frame;
    animCycleFrames.firstFrame = frame;
    animCycleFrames.lastFrame = frame;
    animationFrame = frame;
}

// AS: FoodiniBalloonEffect.resetBalloon — back in Foodini's hand, visible.
void FoodiniBalloonEffect::resetBalloon()
{
    isVisible = true;
    x = gameObj->playerObj->x;
    y = gameObj->playerObj->y + playerAnchorOffset;
    pathY = y;
}

// AS: FoodiniBalloonEffect.floatAway
void FoodiniBalloonEffect::floatAway()
{
    speed = 0;
    isAttached = false;
}

} // namespace papaGame::models::effects
