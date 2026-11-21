#include "game/papaGame/models/objects/GameObject31.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::objects {

// AS: GameObject31.defineVars
void GameObject31::defineVars()
{
    sheetname = "object_spikebed";
    objectname = "Spike Bed";
    sheetIsBitmap = false;
    type = 31;
    spritewidth = 640;
    spriteheight = 90;
    spriteCenterX = 320;
    spriteCenterY = 45;
    spriteTargetX = 16;
    spriteTargetY = 16;
    sheetWidth = 1;
    sheetHeight = 1;
    widthmultiplier = 3;
    heightmultiplier = 3;
    foreground = false;
    collRect = {-16, 0, 64, 48};
    isActivated = true;
    broadcastStyle = 1;
    broadcastReceiptMethod = 3;
    activeCycleFrames = cycleType1("active", 1, 0, 0, 0, 0);
    activatingCycleFrames = cycleType2("activating", 1, 0, -1, {0});
    deactivatingCycleFrames = cycleType2("deactivating", 1, 0, -1, {0});
    inactiveCycleFrames = cycleType1("inactive", 1, 0, 0, 0, 0);
    dirx = 1;
}

// AS: GameObject31.setupParams — [width in tiles (0: to the level's right edge), target row].
void GameObject31::setupParams(const std::vector<as3::Value> &params)
{
    const display::GameDisplay &d = *gameObj->obf_71;
    if (params.size() > 0 && params[0].asNumber() > 0) {
        tilingWidth = params[0].asNumber();
    } else {
        tilingWidth = d.levelTileWidth - xtile;
    }
    if (params.size() > 1 && params[1].asNumber() > 0) {
        targetYtile = params[1].asNumber();
    }
}

// AS: GameObject31.updateObject — triggered (deactivated), it shakes for 16 frames, then rises with dust at both ends
// and a camera jiggle every 30 frames, and a big jiggle when it arrives. A player touching it dies (the camera locks
// where it is) and restarts once they have fallen 3 rows below the screen.
void GameObject31::updateObject()
{
    display::GameDisplay &d = *gameObj->obf_71;
    characters::CustomerChar &p = *gameObj->playerObj;
    managers::EffectManager &fx = *gameObj->obf_V;
    if (isDeactivated) {
        if (ytile > targetYtile) {
            if (delayTimer < delayTimerMax) {
                ++delayTimer;
                if (std::fmod(delayTimer, 2) == 0) {
                    ++y;
                } else {
                    --y;
                }
            } else {
                y -= riseSpeed;
                ytile = std::floor(y / d.tileWidth);
                ++shakeTimer;
                if (std::fmod(shakeTimer, shakeTimerMax) == 0) {
                    gameObj->obf_3Y->setCameraJiggle(4, 4);
                }
                if (std::fmod(shakeTimer, 3) == 0) {
                    fx.addEffect(x - 16, y + 70, "DustEffect");
                    fx.addEffect(x + 624, y + 70, "DustEffect");
                }
            }
        } else if (!isDone) {
            gameObj->obf_3Y->setCameraJiggle(12, 12);
            isDone = true;
        }
    }
    const Rect r{x + collRect.x, y + collRect.y, d.tileWidth * tilingWidth, d.tileWidth * tilingHeight};
    if (p.checkSpriteCollision(r) && !p.isDead) {
        gameObj->obf_3Y->setCameraLock(d.currentXtileLeft, d.currentYtileUp);
        p.hurtPlayer(1, 9999, true);
    } else if (p.isDead && p.ytile > d.currentYtileDown + 3) {
        p.restartPlayer();
    }
    checkSpikingEnemies();
    updateSprite();
}

// AS: GameObject31.checkSpikingEnemies
void GameObject31::checkSpikingEnemies()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    const Rect r{x + collRect.x, y + collRect.y, d.tileWidth * tilingWidth, d.tileWidth * tilingHeight};
    for (Enemy *e : gameObj->obf_E->enemies) {
        if (e && e->checkSpriteCollision(r) && !e->isDead) {
            e->killMe(-1);
        }
    }
}

} // namespace papaGame::models::objects
