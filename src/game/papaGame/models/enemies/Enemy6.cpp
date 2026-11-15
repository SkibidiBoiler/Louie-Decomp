#include "game/papaGame/models/enemies/Enemy6.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy6.defineVars
void Enemy6::defineVars()
{
    type = 6;
    sheetname = "enemysheet_tomato";
    sheetIsBitmap = false;
    height = 15;
    width = 15;
    playerheight = 15;
    spritewidth = 50;
    spriteheight = 54;
    spriteCenterX = 25;
    spriteCenterY = 27;
    spriteTargetX = 25;
    spriteTargetY = 32;
    sheetWidth = 35;
    sheetHeight = 2;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 2;
    walkspeed = 2;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -18;
    flipBlitOffset = 54;
    collRect = {-15, -15, 30, 30};
    gravity = 2;
    normalgravity = 2;
    currentHealth = 1;
    maxHealth = 1;
    facingWalking = true;
    points = 10;
    canBeStunned = true;
    stunDuration = 90;
    enemyCategory = "tomato";
}

// AS: Enemy6.setupParams — [facing, rememberKilled, initial delay, delay after landing, spawner object, (spawned)]
void Enemy6::setupParams(const std::vector<as3::Value> &params)
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
        initialDelay = params[2].asNumber();
    }
    if (params.size() > 3) {
        landDelay = params[3].asNumber();
    }
    if (params.size() > 4) {
        spawnerObjectID = params[4].asNumber();
    }
    if (params.size() > 5) {
        hasJumped = true;
        noticedPlayer = true;
        alwaysNoticePlayer = true;
    }
}

// AS: Enemy6.stunEnemy
void Enemy6::stunEnemy()
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
        // SKELETON: ChallengeManager.recordTag("enemyStun").
        didTagStun = true;
    }
}

// AS: Enemy6.reactToHurtingPlayer
void Enemy6::reactToHurtingPlayer()
{
    isReactingToHurt = true;
    jumpspeed = -10;
    jump = true;
}

// AS: Enemy6.grabAnimationCycle
const AnimationCycle &Enemy6::grabAnimationCycle()
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
        return cycleFrames("squashCycleFrames");
    }
    return cycleFrames("standCycleFrames");
}

// AS: Enemy6.endAnimationCycle
void Enemy6::endAnimationCycle()
{
    if (cycleName == "squash") {
        isSquashing = false;
    } else if (cycleName == "unstun") {
        jumpspeed = -10;
        jump = true;
        isJumpUnstunning = true;
    }
}

// AS: Enemy6.removeEnemy
void Enemy6::removeEnemy()
{
    if (spawnerObjectID != -1) {
        // SKELETON: ObjectManager.getObject(spawnerObjectID, whichScreen).unregisterSpawn() (objects).
    }
    Enemy::removeEnemy();
}

// AS: Enemy6.updateObject — jumps after its delay, squashes on landing.
void Enemy6::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    speed = adjustSpeedForSlopes(walkspeed);
    if (isDead) {
        jumpChar(false);
        moveChar(dirx, 0, 0, false, false, false);
        if (dizzyFXID != -1) {
            // SKELETON: EffectManager.removeEffect(dizzyFXID) (effects).
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
        fallChar();
        if (jump) {
            jumpChar();
        }
        if (!jump) {
            if (wasJumping) {
                isSquashing = true;
            }
            if (!isSquashing) {
                ++delayTimer;
                // QUIRK: delayTimer is never reset and didFirstJump never set, so after the first jump the
                // tomato jumps again as soon as each squash ends.
                if ((!didFirstJump && delayTimer >= initialDelay) || (didFirstJump && delayTimer >= landDelay)) {
                    jump = true;
                    jumpspeed = jumpstart;
                    whichJump = whichJump == 1 ? 2 : 1;
                    if (isReallyOnScreen()) {
                        // SKELETON: jump_tomato1.wav (even id) / jump_tomato2.wav with proximity volume and pan.
                    }
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
        // SKELETON: the DizzyEffect while stunned (effects).
    }
}

// AS: Enemy6.killMe
void Enemy6::killMe(double dir)
{
    Enemy::killMe(dir);
    // SKELETON: ChallengeManager.recordTag("gregtomato") when the player is Greg.
}

} // namespace papaGame::models::enemies
