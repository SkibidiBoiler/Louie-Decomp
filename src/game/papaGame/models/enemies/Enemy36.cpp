#include "game/papaGame/models/enemies/Enemy36.h"

#include <climits>
#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy36.defineVars
void Enemy36::defineVars()
{
    type = 36;
    sheetname = "enemysheet_easyboss";
    sheetIsBitmap = false;
    height = 15;
    width = 15;
    playerheight = 15;
    spritewidth = 145;
    spriteheight = 165;
    spriteCenterX = 72;
    spriteCenterY = 82;
    spriteTargetX = 72;
    spriteTargetY = 130;
    sheetWidth = 10;
    sheetHeight = 10;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 6;
    walkspeed = 6;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -12;
    flipBlitOffset = 825;
    collRect = {-22, -55, 44, 75};
    gravity = 2;
    normalgravity = 2;
    currentHealth = 1;
    maxHealth = 1;
    facingWalking = true;
    points = 10;
    canBeStunned = true;
    stunDuration = 90;
    facingDir = -1;
    walkingDir = -1;
    dirx = -1;
    rememberKilled = true;
}

// AS: Enemy36.killMe — the music stops and the fanfare plays.
void Enemy36::killMe(double dir)
{
    maxjumpvelocity = 12;
    gameObj->obf_7C->playTrack("", INT_MAX, 0, "outin");
    gameObj->obf_7C->playSound("papalouie2_fanfare.wav");
    Enemy::killMe(dir);
}

// AS: Enemy36.stunEnemy
void Enemy36::stunEnemy()
{
    if (!isStunned) {
        isStunned = true;
    } else {
        cycleFrame = 0;
        animationFrame = cycleFrames("stunCycleFrames").frameAt(0);
    }
    if (!didTagStun) {
        gameObj->obf_94->recordTag("enemyStun");
        didTagStun = true;
    }
    isUnstunning = false;
    stunStartTime = 0;
}

// AS: Enemy36.grabAnimationCycle
const AnimationCycle &Enemy36::grabAnimationCycle()
{
    if (isDead) {
        return cycleFrames("dieCycleFrames");
    }
    if (isStunned) {
        return cycleFrames("stunCycleFrames");
    }
    if (isUnstunning) {
        return cycleFrames("unstunCycleFrames");
    }
    if (isBlocking) {
        return cycleFrames("blockCycleFrames");
    }
    if (isAttacking) {
        return cycleFrames("attackCycleFrames");
    }
    if (isTurning) {
        return cycleFrames("turnCycleFrames");
    }
    if (jump) {
        return cycleFrames("jumpCycleFrames");
    }
    return cycleFrames("runCycleFrames"); // walking or not, the original returns the run cycle
}

// AS: Enemy36.endAnimationCycle
void Enemy36::endAnimationCycle()
{
    if (cycleName == "turn") {
        isTurning = false;
    } else if (cycleName == "unstun") {
        isStunned = false;
        isUnstunning = false;
        isWalking = true;
    } else if (cycleName == "attack") {
        isAttacking = false;
        isWalking = true;
    } else if (cycleName == "block") {
        isBlocking = false;
        isWalking = true;
    }
}

// AS: Enemy36.getHit — only a stunned one takes the hit; otherwise he blocks.
bool Enemy36::getHit(double damage, double dir, bool unused, const std::string &weapon)
{
    if (isStunned) {
        return Enemy::getHit(damage, dir, unused, weapon);
    }
    isBlocking = true;
    return false;
}

// AS: Enemy36.updateObject — nothing until the fight is triggered. Blocking or throwing, he slides to a stop (one
// less speed every other frame); the Beaker leaves his hand on the attack cycle's 4th frame. The sprite's anchor
// moves with the facing.
void Enemy36::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    managers::EffectManager &fx = *gameObj->obf_V;
    if (isTriggered) {
        if (isDead) {
            gravity = 1;
            normalgravity = 1;
            jumpChar(false);
            moveChar(dirx, 0, 0, false, false, false);
            if (dizzyFXID != -1) {
                fx.removeEffect(dizzyFXID);
                dizzyFXID = -1;
            }
            updateSprite();
            if (ytile > d.currentYtileDown + 3) {
                removeEnemy();
            } else if (isLaunching && (ytile <= d.currentYtileUp - 1 || ytile < 0)) {
                removeEnemy();
            }
        } else {
            const double oldDir = dirx;
            if (!isStunned && !isUnstunning && !isBlocking && !isAttacking && !jump) {
                speed = adjustSpeedForSlopes(walkspeed);
                moveChar(dirx, 0, 0, true, false);
            } else if (isBlocking || isAttacking) {
                if (speed > 0) {
                    if (std::fmod(gameObj->obf_8o->gameplayTimer, 2) == 0) {
                        --speed;
                    }
                    moveChar(dirx, 0, 0, true, false, true);
                }
            }
            fallChar();
            if (jump) {
                jumpChar();
            }
            facingDir = dirx;
            walkingDir = dirx;
            if (dirx != oldDir && !isStunned && !isUnstunning && !isBlocking && !isAttacking) {
                isTurning = true;
            }
            if (isStunned) {
                ++stunStartTime;
                if (stunStartTime >= stunDuration) {
                    isStunned = false;
                    isUnstunning = true;
                }
            }
            if (!isStunned && !isUnstunning && !isBlocking && !jump) {
                if (!isAttacking) {
                    ++attackTimer;
                    if (attackTimer > attackTimerMax) {
                        isAttacking = true;
                        attackTimer = 0;
                    }
                } else if (cycleFrame == 3) {
                    gameObj->obf_4l->addBullet(x + 30 * facingDir, y - 30, facingDir, 0, "Beaker", "", true, 1, true);
                }
            }
            updateSprite();
            if (dizzyFXID == -1 && isStunned && cycleFrame == cycleFrames("stunCycleFrames").loopFrame) {
                dizzyFXID = fx.addEffect(x, y - 30, "DizzyEffect", facingDir == 1 ? "faceRight" : "faceLeft");
            } else if (!isStunned && dizzyFXID != -1) {
                fx.removeEffect(dizzyFXID);
                dizzyFXID = -1;
            }
        }
    }
    spriteTargetX = facingDir == 1 ? 70 : 75;
}

// AS: Enemy36.sendTriggered — the "Start Fight" trigger starts him and the boss music.
void Enemy36::sendTriggered(double /*state*/, bool /*pan*/, bool /*fromGlobal*/)
{
    isTriggered = true;
    gameObj->obf_7C->playTrack("BossTrack", INT_MAX, 0, "crossfade");
}

} // namespace papaGame::models::enemies
