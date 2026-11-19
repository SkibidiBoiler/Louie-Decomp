#include "game/papaGame/models/enemies/Enemy3.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy3.defineVars
void Enemy3::defineVars()
{
    type = 3;
    sheetname = "enemysheet_fortonion";
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
    sheetWidth = 28;
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

// AS: Enemy3.setupParams — [facing, rememberKilled, spawner object, (spawned)]
void Enemy3::setupParams(const std::vector<as3::Value> &params)
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

// AS: Enemy3.stunEnemy — a stun makes it collapse (again, if it already wears its helmet low).
void Enemy3::stunEnemy()
{
    if (!isStunned) {
        isStunned = true;
        isCollapsing = true;
    } else {
        cycleFrame = 0;
        animationFrame = cycleFrames("restompCycleFrames").frameAt(0);
        isStunned = true;
        isCollapsing = true;
    }
    if (!didTagStun) {
        gameObj->obf_94->recordTag("enemyStun");
        didTagStun = true;
    }
}

// AS: Enemy3.reactToHurtingPlayer — collapsed, it turns to run at the player; otherwise it hops.
void Enemy3::reactToHurtingPlayer()
{
    if (isStomped) {
        if (gameObj->playerObj->x < x) {
            dirx = 1;
            facingDir = 1;
            walkingDir = 1;
        } else {
            dirx = -1;
            facingDir = -1;
            walkingDir = -1;
        }
    } else {
        isReactingToHurt = true;
        jumpspeed = -10;
        jump = true;
    }
}

// AS: Enemy3.grabAnimationCycle
const AnimationCycle &Enemy3::grabAnimationCycle()
{
    if (isDead) {
        return cycleFrames(isStomped ? "diestompedCycleFrames" : "dieCycleFrames");
    }
    if (isCollapsing) {
        return cycleFrames(isStomped ? "restompCycleFrames" : "stompCycleFrames");
    }
    if (isStomped) {
        return cycleFrames("walkstompedCycleFrames");
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

// AS: Enemy3.endAnimationCycle
void Enemy3::endAnimationCycle()
{
    if (cycleName == "turn") {
        isTurning = false;
    } else if (cycleName == "stomp" || cycleName == "restomp") {
        isCollapsing = false;
        isStomped = true;
        isStunned = false;
    }
}

// AS: Enemy3.removeEnemy
void Enemy3::removeEnemy()
{
    if (spawnerObjectID != -1) {
        if (GameObject *spawner = gameObj->obf_7u->getObject(spawnerObjectID, whichScreen)) {
            spawner->unregisterSpawn();
        }
    }
    Enemy::removeEnemy();
}

// AS: Enemy3.updateObject
void Enemy3::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    speed = adjustSpeedForSlopes(isStomped ? stompedSpeed : walkspeed);
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
        if (!isStunned && !isUnstunning && !isCollapsing && !isReactingToHurt) {
            if (!isStomped) {
                moveChar(dirx, 0, 0, true, false);
            } else {
                moveChar(dirx, 0, 0, true, true);
                if (static_cast<int>(cycleFrame) % 8 == 0) {
                    gameObj->obf_V->addEffect(x - 6 * facingDir, y + 8, "DustEffect");
                }
            }
        }
        fallChar();
        if (jump) {
            jumpChar();
        }
        facingDir = dirx;
        walkingDir = dirx;
        if (dirx != lastDirX && !isStomped) {
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
