#include "game/papaGame/models/enemies/Enemy33.h"

#include <algorithm>
#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy33.Enemy33 — it heads for its target column first, waving about its starting height.
void Enemy33::init(double xtile_, double ytile_, double id_, double whichScreen_, double whichTileset_,
                   double screenObjID_, const std::vector<as3::Value> &extraParams)
{
    Enemy::init(xtile_, ytile_, id_, whichScreen_, whichTileset_, screenObjID_, extraParams);
    startingXtile = xtile;
    initialY = y;
    dirx = startingXtile > targetXtile ? -1 : 1;
}

// AS: Enemy33.defineVars
void Enemy33::defineVars()
{
    type = 33;
    sheetname = "enemysheet_awesomesaucer";
    sheetIsBitmap = false;
    height = 15;
    width = 15;
    playerheight = 15;
    spritewidth = 76;
    spriteheight = 64;
    spriteCenterX = 38;
    spriteCenterY = 32;
    spriteTargetX = 38;
    spriteTargetY = 48;
    sheetWidth = 31;
    sheetHeight = 1;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 4;
    walkspeed = 4;
    diry = 0;
    jumpspeed = 0;
    jumpstart = -16;
    flipBlitOffset = 0;
    collRect = {-15, -20, 30, 30};
    gravity = 2;
    normalgravity = 2;
    currentHealth = 1;
    maxHealth = 1;
    facingWalking = true;
    points = 10;
    canBeStunned = true;
    stunDuration = 90;
    isFlying = true;
    enemyCategory = "saucer";
}

// AS: Enemy33.setupParams — [facing, rememberKilled, target column, frames between shots, starting shot timer].
void Enemy33::setupParams(const std::vector<as3::Value> &params)
{
    if (params.size() > 0) {
        facingDir = params[0].asNumber();
        walkingDir = facingDir;
        dirx = walkingDir;
    }
    if (params.size() > 1) {
        rememberKilled = params[1].asNumber() == 1;
    }
    if (params.size() > 2) {
        targetXtile = params[2].asNumber();
    }
    if (params.size() > 3) {
        delayBeforeShooting = params[3].asNumber();
    }
    if (params.size() > 4) {
        shootTimer = params[4].asNumber();
    }
}

// AS: Enemy33.stunEnemy — it stops dead while it spins.
void Enemy33::stunEnemy()
{
    if (!isStunned) {
        isStunned = true;
    } else {
        cycleFrame = 0;
        animationFrame = cycleFrames("stunCycleFrames").frameAt(0);
    }
    isUnstunning = false;
    isJumpUnstunning = false;
    stunStartTime = 0;
    speed = 0;
}

// AS: Enemy33.grabAnimationCycle
const AnimationCycle &Enemy33::grabAnimationCycle()
{
    if (isDead) {
        return cycleFrames(dirx == 1 ? "dieRightCycleFrames" : "dieLeftCycleFrames");
    }
    if (isStunned) {
        return cycleFrames("stunCycleFrames");
    }
    return cycleFrames("flyCycleFrames");
}

// AS: Enemy33.endAnimationCycle — the stun lasts one spin, then it starts again slowly.
void Enemy33::endAnimationCycle()
{
    if (cycleName == "stun") {
        isStunned = false;
        speed = 1;
    }
}

// AS: Enemy33.updateObject — it slows by one every other frame in its last column and turns on its centre, then
// speeds up again to walkspeed; it shoots every delayBeforeShooting frames unless stunned.
void Enemy33::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    const double timer = gameObj->obf_8o->gameplayTimer;
    if (isDead) {
        speed = walkspeed;
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
    const double oldDir = dirx;
    if (!isStunned) {
        moveChar(dirx, 0, 0, true);
    }
    const double rightTile = std::max(startingXtile, targetXtile);
    const double leftTile = std::min(startingXtile, targetXtile);
    if (dirx == 1 && xtile >= rightTile) {
        if (x > rightTile * d.tileWidth + d.tileWidth / 2) {
            x = rightTile * d.tileWidth + d.tileWidth / 2;
            xtile = std::floor(x / d.tileWidth);
            dirx = -1;
            speed = 0;
        } else if (speed > 1 && std::fmod(timer, 2) == 0) {
            --speed;
        }
    } else if (dirx == -1 && xtile <= leftTile) {
        if (x < leftTile * d.tileWidth + d.tileWidth / 2) {
            x = leftTile * d.tileWidth + d.tileWidth / 2;
            xtile = std::floor(x / d.tileWidth);
            dirx = 1;
            speed = 0;
        } else if (speed > 1 && std::fmod(timer, 2) == 0) {
            --speed;
        }
    } else if (speed < walkspeed) {
        if (std::fmod(timer, 2) == 0) {
            ++speed;
        }
    }
    facingDir = dirx;
    walkingDir = dirx;
    if (dirx != oldDir) {
        isTurning = true;
    }
    ++waveTimer;
    y = initialY + std::sin(waveTimer / wavePeriod) * waveAmplitude;
    ytile = std::floor(y / d.tileWidth);
    if (!isStunned) {
        ++shootTimer;
        if (shootTimer >= delayBeforeShooting) {
            shootSauce();
            shootTimer = 0;
        }
    }
    updateSprite();
}

// AS: Enemy33.shootSauce — a drop straight down from 12 px below its centre.
void Enemy33::shootSauce()
{
    gameObj->obf_4l->addBullet(x, y + 12, 0, 1, "SauceBullet", "", true, 1, true);
    gameObj->obf_7C->playSound("battle_bow1.wav");
}

} // namespace papaGame::models::enemies
