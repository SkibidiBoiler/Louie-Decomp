#include "game/papaGame/models/enemies/Enemy5.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy5.defineVars
void Enemy5::defineVars()
{
    type = 5;
    sheetname = "enemysheet_kaiseronion";
    sheetIsBitmap = false;
    height = 15;
    width = 15;
    playerheight = 15;
    spritewidth = 56;
    spriteheight = 76;
    spriteCenterX = 28;
    spriteCenterY = 38;
    spriteTargetX = 32;
    spriteTargetY = 56;
    sheetWidth = 35;
    sheetHeight = 2;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 2;
    walkspeed = 2;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -12;
    flipBlitOffset = 76;
    collRect = {-15, -20, 30, 40};
    gravity = 2;
    normalgravity = 2;
    currentHealth = 1;
    maxHealth = 1;
    facingWalking = true;
    points = 10;
    canBeStunned = false;
    enemyCategory = "onion";
}

// AS: Enemy5.setupParams — [facing, rememberKilled, spawner object, (spawned)]
void Enemy5::setupParams(const std::vector<as3::Value> &params)
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
        spawnerObjectID = params[2].asNumber();
    }
    if (params.size() > 3) {
        hasJumped = true;
        noticedPlayer = true;
        alwaysNoticePlayer = true;
    }
}

// AS: Enemy5.reactToHurtingPlayer — it hops.
void Enemy5::reactToHurtingPlayer()
{
    isReactingToHurt = true;
    jumpspeed = -10;
    jump = true;
}

// AS: Enemy5.grabAnimationCycle
const AnimationCycle &Enemy5::grabAnimationCycle()
{
    if (isDead) {
        return cycleFrames("dieCycleFrames");
    }
    if (jump && jumpspeed < 0) {
        return cycleFrames("jumpCycleFrames");
    }
    if (jump && jumpspeed >= 0) {
        return cycleFrames("fallCycleFrames");
    }
    if (isTurning) {
        return cycleFrames("turnCycleFrames");
    }
    if (isWalking) {
        return cycleFrames("walkCycleFrames");
    }
    return cycleFrames("jumpCycleFrames");
}

// AS: Enemy5.endAnimationCycle
void Enemy5::endAnimationCycle()
{
    if (cycleName == "turn") {
        isTurning = false;
    }
}

// AS: Enemy5.removeEnemy — QUIRK: unlike Enemy3 it does not unregister from its spawner.
void Enemy5::removeEnemy()
{
    Enemy::removeEnemy();
}

// AS: Enemy5.updateObject
void Enemy5::updateObject()
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
    } else if (checkOnScreen() || spawnerObjectID != -1) {
        const double lastDirX = dirx;
        if (!isStunned && !isUnstunning && !isReactingToHurt) {
            moveChar(dirx, 0, 0, true, false);
        }
        fallChar();
        if (jump) {
            jumpChar();
        }
        facingDir = dirx;
        walkingDir = dirx;
        if (dirx != lastDirX) {
            isTurning = true;
        }
        if (isReactingToHurt && !jump) {
            isReactingToHurt = false;
        }
        updateSprite();
    }
    spriteTargetX = facingDir == 1 ? 32 : 24;
}

} // namespace papaGame::models::enemies
