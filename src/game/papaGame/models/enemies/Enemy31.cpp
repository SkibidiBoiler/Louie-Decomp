#include "game/papaGame/models/enemies/Enemy31.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy31.defineVars
void Enemy31::defineVars()
{
    type = 31;
    sheetname = "enemysheet_cheesewheel";
    sheetIsBitmap = false;
    height = 15;
    width = 15;
    playerheight = 15;
    spritewidth = 60;
    spriteheight = 60;
    spriteCenterX = 30;
    spriteCenterY = 30;
    spriteTargetX = 30;
    spriteTargetY = 44;
    sheetWidth = 30;
    sheetHeight = 1;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 5;
    walkspeed = 5;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -12;
    flipBlitOffset = 0;
    collRect = {-15, -30, 30, 40};
    gravity = 2;
    normalgravity = 2;
    currentHealth = 1;
    maxHealth = 1;
    facingWalking = true;
    points = 10;
    canBeStunned = true;
    stunDuration = 90;
    rememberKilled = false;
    bounceY = true;
    bounceDecayY = 0.5;
    enemyCategory = "cheese";
}

// AS: Enemy31.setupParams — [direction, spawner object].
void Enemy31::setupParams(const std::vector<as3::Value> &params)
{
    if (params.size() > 0) {
        facingDir = params[0].asNumber();
        walkingDir = facingDir;
        dirx = walkingDir;
    }
    if (params.size() > 1) {
        spawnerObjectID = params[1].asNumber();
    }
}

// AS: Enemy31.stunEnemy — a stomp is a hit.
void Enemy31::stunEnemy()
{
    getHit();
}

// AS: Enemy31.grabAnimationCycle — one frame forward (or back) per frame, round the 30-frame wheel.
const AnimationCycle &Enemy31::grabAnimationCycle()
{
    double &frame = animCycleFrames.frames[0];
    if (dirx == 1) {
        frame += 1;
    } else {
        frame = frame - 1;
    }
    if (frame >= totalFrames) {
        frame -= totalFrames;
    } else if (frame < 0) {
        frame += totalFrames;
    }
    return animCycleFrames;
}

// AS: Enemy31.removeEnemy
void Enemy31::removeEnemy()
{
    if (spawnerObjectID != -1) {
        if (GameObject *spawner = gameObj->obf_7u->getObject(spawnerObjectID, whichScreen)) {
            spawner->unregisterSpawn();
        }
    }
    Enemy::removeEnemy();
}

// AS: Enemy31.updateObject
void Enemy31::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    speed = adjustSpeedForSlopes(walkspeed);
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
    moveChar(dirx, 0, 0, true, false);
    fallChar();
    if (jump) {
        jumpChar();
    }
    facingDir = dirx;
    walkingDir = dirx;
    updateSprite();
}

} // namespace papaGame::models::enemies
