#include "game/papaGame/models/objects/GameObject27.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::objects {

// AS: GameObject27.defineVars
void GameObject27::defineVars()
{
    sheetname = "object_bbqsauce";
    objectname = "BBQ Quicksauce";
    sheetIsBitmap = false;
    type = 27;
    spritewidth = 64;
    spriteheight = 64;
    spriteCenterX = 32;
    spriteCenterY = 32;
    spriteTargetX = 16;
    spriteTargetY = 16;
    sheetWidth = 29;
    sheetHeight = 2;
    widthmultiplier = 3;
    heightmultiplier = 3;
    foreground = true;
    collRect = {-16, -16, 64, 64};
    isTilingObject = true;
    tilingHasCap = true;
    tilingCapBlitOffsetY = 0;
    tilingNoCapBlitOffsetY = 64;
    tilingUseColorFill = false;
    isActivated = true;
    broadcastStyle = 1;
    activeCycleFrames = cycleType1("active", 1, 0, 0, 0, 28);
    activatingCycleFrames = cycleType2("activating", 1, 0, -1, {0});
    deactivatingCycleFrames = cycleType2("deactivating", 1, 0, -1, {0});
    inactiveCycleFrames = cycleType1("inactive", 1, 0, 0, 0, 0);
    dirx = 1;
}

// AS: GameObject27.setupParams — [width in tiles, height in tiles]; 0 or missing reaches the level's edge.
void GameObject27::setupParams(const std::vector<as3::Value> &params)
{
    const display::GameDisplay &d = *gameObj->obf_71;
    tilingWidth = params.size() > 0 && params[0].asNumber() > 0 ? params[0].asNumber() : d.levelTileWidth - xtile;
    tilingHeight = params.size() > 1 && params[1].asNumber() > 0 ? params[1].asNumber() : d.levelTileHeight - ytile;
}

void GameObject27::splash(double px)
{
    managers::EffectManager &fx = *gameObj->obf_V;
    if (type == 27) {
        fx.addEffect(px, y, "BBQBitEffect", "piece1");
        fx.addEffect(px - 3, y, "BBQBitEffect", "piece2");
        fx.addEffect(px - 6, y, "BBQBitEffect", "piece1");
        fx.addEffect(px + 4, y, "BBQBitEffect", "piece3");
    } else if (type == 32) {
        fx.addEffect(px, y, "RainbowBitEffect", "piece1");
        fx.addEffect(px - 3, y, "RainbowBitEffect", "piece2");
        fx.addEffect(px - 6, y, "RainbowBitEffect", "piece3");
        fx.addEffect(px + 4, y, "SauceBitEffect", "piece1");
        fx.addEffect(px, y, "BlueberryBitEffect", "piece1");
    }
}

// AS: GameObject27.updateObject — the player's feet (-16,-10,32,24) in the pool: a splash when they land in it,
// then they sink at 1 px a frame and walk at speed 1; a splash when they jump out of the surface. Leaving the pool
// ends the quicksand.
void GameObject27::updateObject()
{
    characters::CustomerChar &p = *gameObj->playerObj;
    const display::GameDisplay &d = *gameObj->obf_71;
    const Rect area{x + collRect.x, y + collRect.y, d.tileWidth * tilingWidth, d.tileWidth * tilingHeight};
    const Rect feet{-16, -10, 32, 24};
    if (p.checkSpriteCollision(area, &feet)) {
        if (!wasMovingPlayer && p.jump) {
            splash(p.x);
            gameObj->obf_7C->playSound("getcheesed.mp3", false, 0.7);
        }
        if (p.jump && p.jumpspeed >= 0) {
            p.jump = false;
            p.isGliding = false;
            p.isDoubleJumping = false;
            p.isGroundPounding = false;
        }
        wasMovingPlayer = true;
        p.isSliding = false;
        p.isInQuicksand = true;
        p.setSpeed(1);
        p.moveChar(0, 1, 0);
        if (p.jump && p.jumpspeed < 0 && !wasPlayerJumping) {
            splash(p.x);
            wasPlayerJumping = true;
        } else if (!p.jump && wasPlayerJumping) {
            wasPlayerJumping = false;
        }
    } else if (wasMovingPlayer) {
        p.isInQuicksand = false;
        p.isWalking = false;
        wasMovingPlayer = false;
        if (playerSplashID > -1) {
            gameObj->obf_V->removeEffect(playerSplashID);
            playerSplashID = -1;
        }
    }
    checkEnemiesInWater();
    updateSprite();
}

// AS: GameObject27.checkEnemiesInWater — any enemy but the X Zone bosses (38, 39) that touches the pool dies in it.
void GameObject27::checkEnemiesInWater()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    const Rect area{x + collRect.x, y + collRect.y, d.tileWidth * tilingWidth, d.tileWidth * tilingHeight};
    auto &enemies = gameObj->obf_E->enemies;
    for (size_t i = 0; i < enemies.size(); ++i) {
        Enemy *e = enemies[i];
        if (e && e->checkSpriteCollision(area) && !e->isDead && e->type != 38 && e->type != 39) {
            splash(e->x);
            e->killMe(-1);
            gameObj->obf_7C->playSound("getcheesed.mp3", false, 0.7 * e->getProximityVolume(), e->getProximityPan());
            gameObj->obf_94->recordTag("waterEnemy");
        }
    }
}

} // namespace papaGame::models::objects
