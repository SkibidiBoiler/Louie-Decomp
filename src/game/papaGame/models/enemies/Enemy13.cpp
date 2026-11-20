#include "game/papaGame/models/enemies/Enemy13.h"

#include <cmath>

#include "game/obf_AF/obf_1n.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy13.Enemy13 — its starting point is the centre of its circle.
void Enemy13::init(double xtile_, double ytile_, double id_, double whichScreen_, double whichTileset_,
                   double screenObjID_, const std::vector<as3::Value> &extraParams)
{
    Enemy::init(xtile_, ytile_, id_, whichScreen_, whichTileset_, screenObjID_, extraParams);
    flightCircleX = x;
    flightCircleY = y;
}

// AS: Enemy13.defineVars
void Enemy13::defineVars()
{
    type = 13;
    sheetname = "enemysheet_brussellark";
    sheetIsBitmap = false;
    height = 15;
    width = 15;
    playerheight = 15;
    spritewidth = 68;
    spriteheight = 50;
    spriteCenterX = 34;
    spriteCenterY = 25;
    spriteTargetX = 34;
    spriteTargetY = 25;
    sheetWidth = 22;
    sheetHeight = 1;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 4;
    walkspeed = 4;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -12;
    flipBlitOffset = 0;
    collRect = {-15, -15, 30, 30};
    gravity = 2;
    normalgravity = 2;
    currentHealth = 1;
    maxHealth = 1;
    facingWalking = true;
    points = 10;
    canBeStunned = true;
    stunDuration = 90;
    enemyCategory = "lark";
}

// AS: Enemy13.checkOnScreen — the whole circle (2 tiles of margin) counts.
bool Enemy13::checkOnScreen()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    const double cx = std::floor(flightCircleX / d.tileWidth);
    const double cy = std::floor(flightCircleY / d.tileWidth);
    return cx + tileRadius > d.currentXtileLeft - 2 && cx - tileRadius < d.currentXtileRight + 2 &&
           cy + tileRadius > d.currentYtileUp - 2 && cy - tileRadius < d.currentYtileDown + 2;
}

// AS: Enemy13.setupParams — [rememberKilled, radius in tiles, direction (1 / -1), starting angle in degrees].
void Enemy13::setupParams(const std::vector<as3::Value> &params)
{
    auto param = [&](size_t i) { return i < params.size() ? params[i].asNumber() : as3::NaN; };
    if (params.size() > 0) {
        rememberKilled = param(0) == 1;
    }
    tileRadius = param(1);
    rotationDirection = param(2);
    blimpAngle = obf_AF::obf_1n::degreesToRads(param(3));
    rotationTimer = blimpAngle / (walkspeed / (tileRadius * gameObj->obf_71->tileWidth));
}

// AS: Enemy13.stunEnemy — a stomp is a hit, knocking it away from the player.
void Enemy13::stunEnemy()
{
    getHit(1, gameObj->playerObj->x > x ? -1 : 1, false);
}

// AS: Enemy13.grabAnimationCycle
const AnimationCycle &Enemy13::grabAnimationCycle()
{
    return cycleFrames(isDead ? "dieCycleFrames" : "flyCycleFrames");
}

// AS: Enemy13.removeEnemy
void Enemy13::removeEnemy()
{
    if (spawnerObjectID != -1) {
        if (GameObject *spawner = gameObj->obf_7u->getObject(spawnerObjectID, whichScreen)) {
            spawner->unregisterSpawn();
        }
    }
    Enemy::removeEnemy();
}

// AS: Enemy13.updateObject — flaps (with a sound on frames 4 and 13) round its circle.
void Enemy13::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    if (isDead) {
        jumpChar(false);
        moveChar(dirx, 0, 0, false, false, false);
        updateSprite();
        if (ytile > d.currentYtileDown + 1) {
            removeEnemy();
        } else if (isLaunching && (ytile <= d.currentYtileUp - 1 || ytile < 0)) {
            removeEnemy();
        }
        return;
    }
    if (!checkOnScreen() && spawnerObjectID == -1) {
        return;
    }
    ++rotationTimer;
    flyChar();
    updateSprite();
    if (animationFrame == 4) {
        if (isReallyOnScreen()) {
            if (lastFlap == 1) {
                gameObj->obf_7C->playSound("tinywingflap2.wav", false, getProximityVolume(), getProximityPan());
                lastFlap = 2;
            } else {
                gameObj->obf_7C->playSound("tinywingflap1.wav", false, getProximityVolume(), getProximityPan());
                lastFlap = 1;
            }
        }
    } else if (animationFrame == 13) {
        if (isReallyOnScreen()) {
            gameObj->obf_7C->playSound("wingupflap.wav", false, getProximityVolume(), getProximityPan());
        }
    }
}

// AS: Enemy13.flyChar — the point on the circle for rotationTimer (walkspeed px per frame), kept inside the level.
void Enemy13::flyChar()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    const double tile = d.tileWidth;
    const double radius = tileRadius * tile;
    const double angularSpeed = walkspeed * rotationDirection / radius;
    x = as3::round(flightCircleX + radius * std::cos(angularSpeed * rotationTimer));
    y = as3::round(flightCircleY + radius * std::sin(angularSpeed * rotationTimer));
    const auto &tiles = gameObj->obf_AQ->currentScreenData->tileArray;
    const double levelW = static_cast<double>(tiles[0].size()) * tile;
    const double levelH = static_cast<double>(tiles.size()) * tile;
    if (x < 0) {
        x = 0;
    }
    if (x > levelW) {
        x = levelW;
    }
    if (y < 0) {
        y = 0;
    }
    if (y > levelH) {
        y = levelH;
    }
    xtile = std::floor(x / tile);
    ytile = std::floor(y / tile);
}

} // namespace papaGame::models::enemies
