#include "game/papaGame/models/enemies/Enemy1.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy1.defineVars
void Enemy1::defineVars()
{
    type = 1;
    sheetname = "enemysheet_onion";
    sheetIsBitmap = false;
    height = 15;
    width = 15;
    playerheight = 15;
    spritewidth = 56;
    spriteheight = 66;
    spriteCenterX = 28;
    spriteCenterY = 33;
    spriteTargetX = 32;
    spriteTargetY = 46;
    sheetWidth = 26;
    sheetHeight = 4;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 2;
    walkspeed = 2;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -12;
    flipBlitOffset = 132;
    collRect = {-15, -20, 30, 40};
    gravity = 2;
    normalgravity = 2;
    currentHealth = 1;
    maxHealth = 1;
    facingWalking = true;
    points = 10;
    canBeStunned = true;
    stunDuration = 90;
    enemyCategory = "onion";
}

// AS: Enemy1.setupParams — [facing, rememberKilled, spawner object, (spawned: chase the player)]
void Enemy1::setupParams(const std::vector<as3::Value> &params)
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

// AS: Enemy1.stunEnemy
void Enemy1::stunEnemy()
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
    if (!didTagStun) {
        gameObj->obf_94->recordTag("enemyStun");
        didTagStun = true;
    }
}

// AS: Enemy1.reactToHurtingPlayer
void Enemy1::reactToHurtingPlayer()
{
    isReactingToHurt = true;
    jumpspeed = -10;
    jump = true;
}

// AS: Enemy1.grabAnimationCycle
const AnimationCycle &Enemy1::grabAnimationCycle()
{
    if (isDead) {
        return cycleFrames("dieCycleFrames");
    }
    if (isStunned) {
        return cycleFrames("stunCycleFrames");
    }
    if (jump && jumpspeed < 0) {
        return cycleFrames("jumpCycleFrames");
    }
    if (jump && jumpspeed >= 0) {
        return cycleFrames("fallCycleFrames");
    }
    if (isUnstunning) {
        return cycleFrames("unstunCycleFrames");
    }
    if (isTurning) {
        return cycleFrames("turnCycleFrames");
    }
    if (isWalking) {
        return cycleFrames("walkCycleFrames");
    }
    return cycleFrames("standCycleFrames");
}

// AS: Enemy1.endAnimationCycle
void Enemy1::endAnimationCycle()
{
    if (cycleName == "turn") {
        isTurning = false;
    } else if (cycleName == "unstun") {
        jumpspeed = -10;
        jump = true;
        isJumpUnstunning = true;
    }
}

// AS: Enemy1.removeEnemy
void Enemy1::removeEnemy()
{
    if (spawnerObjectID != -1) {
        if (GameObject *spawner = gameObj->obf_7u->getObject(spawnerObjectID, whichScreen)) {
            spawner->unregisterSpawn();
        }
    }
    Enemy::removeEnemy();
}

// AS: Enemy1.updateObject
void Enemy1::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    speed = adjustSpeedForSlopes(walkspeed);
    if (isDead) {
        jumpChar(false);
        moveChar(dirx, 0, 0, false, false, false);
        if (dizzyFXID != -1) {
            gameObj->obf_V->removeEffect(dizzyFXID);
            dizzyFXID = -1;
        }
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
        if (isStunned) {
            ++stunStartTime;
            if (stunStartTime >= stunDuration) {
                isStunned = false;
                isUnstunning = true;
            }
        } else if (isUnstunning && isJumpUnstunning && !jump) {
            isUnstunning = false;
            isJumpUnstunning = false;
        }
        if (isReactingToHurt && !jump) {
            isReactingToHurt = false;
        }
        updateSprite();
        if (dizzyFXID == -1 && isStunned && cycleFrame == cycleFrames("stunCycleFrames").loopFrame) {
            dizzyFXID = gameObj->obf_V->addEffect(x, y - 30, "DizzyEffect", facingDir == 1 ? "faceRight" : "faceLeft");
        } else if (!isStunned && dizzyFXID != -1) {
            gameObj->obf_V->removeEffect(dizzyFXID);
            dizzyFXID = -1;
        }
    }
    spriteTargetX = facingDir == 1 ? 32 : 24;
}

} // namespace papaGame::models::enemies
