#include "game/papaGame/models/enemies/Enemy7.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy7.defineVars
void Enemy7::defineVars()
{
    type = 7;
    sheetname = "enemysheet_romatomato";
    sheetIsBitmap = false;
    height = 15;
    width = 15;
    playerheight = 15;
    spritewidth = 46;
    spriteheight = 60;
    spriteCenterX = 23;
    spriteCenterY = 30;
    spriteTargetX = 23;
    spriteTargetY = 37;
    sheetWidth = 34;
    sheetHeight = 2;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 2;
    walkspeed = 2;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -12;
    flipBlitOffset = 60;
    collRect = {-15, -15, 30, 30};
    gravity = 1;
    normalgravity = 1;
    currentHealth = 1;
    maxHealth = 1;
    facingWalking = true;
    points = 10;
    canBeStunned = true;
    stunDuration = 90;
    enemyCategory = "tomato";
}

// AS: Enemy7.setupParams — [facing, rememberKilled, spawner object, (spawned)]
void Enemy7::setupParams(const std::vector<as3::Value> &params)
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

// AS: Enemy7.stunEnemy
void Enemy7::stunEnemy()
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
    jumpspeed = 0;
    if (!didTagStun) {
        gameObj->obf_94->recordTag("enemyStun");
        didTagStun = true;
    }
}

// AS: Enemy7.reactToHurtingPlayer
void Enemy7::reactToHurtingPlayer()
{
    isReactingToHurt = true;
    jumpspeed = -10;
    jump = true;
}

// AS: Enemy7.grabAnimationCycle
const AnimationCycle &Enemy7::grabAnimationCycle()
{
    if (isDead) {
        return cycleFrames("dieCycleFrames");
    }
    if (isStunned) {
        return cycleFrames("stunCycleFrames");
    }
    if (jump && jumpspeed < 0) {
        return cycleFrames("jump" + as3::String(whichJump) + "CycleFrames");
    }
    if (jump && jumpspeed >= 0) {
        return cycleFrames("fall" + as3::String(whichJump) + "CycleFrames");
    }
    if (isUnstunning) {
        return cycleFrames("unstunCycleFrames");
    }
    if (isSquashing) {
        return cycleFrames("land" + as3::String(whichJump) + "CycleFrames");
    }
    return cycleFrames("jump1CycleFrames");
}

// AS: Enemy7.endAnimationCycle
void Enemy7::endAnimationCycle()
{
    if (cycleName == "land1" || cycleName == "land2") {
        isSquashing = false;
    } else if (cycleName == "unstun") {
        jumpspeed = -10;
        jump = true;
        isJumpUnstunning = true;
    }
}

// AS: Enemy7.removeEnemy
void Enemy7::removeEnemy()
{
    if (spawnerObjectID != -1) {
        if (GameObject *spawner = gameObj->obf_7u->getObject(spawnerObjectID, whichScreen)) {
            spawner->unregisterSpawn();
        }
    }
    Enemy::removeEnemy();
}

// AS: Enemy7.updateObject — it hops forward without a pause, turning at walls, and squashes on landing.
void Enemy7::updateObject()
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
        const bool wasJumping = jump;
        if (!isSquashing) {
            moveChar(dirx, 0, 0, true);
        }
        fallChar();
        if (jump) {
            jumpChar();
        }
        facingDir = dirx;
        walkingDir = dirx;
        if (!jump) {
            if (wasJumping) {
                isSquashing = true;
            }
            if (!isSquashing) {
                jump = true;
                jumpspeed = jumpstart;
                whichJump = whichJump == 1 ? 2 : 1;
                if (isReallyOnScreen()) {
                    gameObj->obf_7C->playSound("jump_roma.wav", false, getProximityVolume(), getProximityPan());
                }
            }
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
}

// AS: Enemy7.killMe
void Enemy7::killMe(double dir)
{
    Enemy::killMe(dir);
    if (gameObj->playerObj && gameObj->playerObj->playerData() && gameObj->playerObj->playerData()->customerName == "Greg") {
        gameObj->obf_94->recordTag("gregtomato");
    }
}

} // namespace papaGame::models::enemies
