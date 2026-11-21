#include "game/papaGame/models/enemies/Enemy30.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy30.defineVars
void Enemy30::defineVars()
{
    type = 30;
    sheetname = "enemysheet_radishoot";
    sheetIsBitmap = false;
    height = 15;
    width = 15;
    playerheight = 15;
    spritewidth = 64;
    spriteheight = 115;
    spriteCenterX = 32;
    spriteCenterY = 57;
    spriteTargetX = 32;
    spriteTargetY = 80;
    sheetWidth = 25;
    sheetHeight = 2;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 2;
    walkspeed = 2;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -10;
    flipBlitOffset = 0;
    collRect = {-11, -10, 22, 20};
    gravity = 1;
    normalgravity = 1;
    currentHealth = 1;
    maxHealth = 1;
    facingWalking = true;
    points = 10;
    canBeStunned = true;
    stunDuration = 90;
    rememberKilled = false;
    enemyCategory = "radish";
}

// AS: Enemy30.setupParams — [spawner object id].
void Enemy30::setupParams(const std::vector<as3::Value> &params)
{
    if (params.size() > 0) {
        spawnerObjectID = params[0].asNumber();
    }
}

// AS: Enemy30.stunEnemy — not while it folds its parachute; stunned in the air, it loses the parachute.
void Enemy30::stunEnemy()
{
    if (isEndFlying) {
        return;
    }
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
    if (isParachuting) {
        isParachuting = false;
        isEndFlying = false;
    }
}

// AS: Enemy30.reactToHurtingPlayer
void Enemy30::reactToHurtingPlayer()
{
    isReactingToHurt = true;
    jumpspeed = -10;
    jump = true;
}

// AS: Enemy30.grabAnimationCycle
const AnimationCycle &Enemy30::grabAnimationCycle()
{
    if (isDead) {
        return cycleFrames("dieCycleFrames");
    }
    if (isParachuting) {
        return cycleFrames("flyCycleFrames");
    }
    if (isEndFlying) {
        return cycleFrames("endFlyingCycleFrames");
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
    if (isSquashing) {
        return cycleFrames("landCycleFrames");
    }
    if (isUnstunning) {
        return cycleFrames("unstunCycleFrames");
    }
    return cycleFrames("standCycleFrames");
}

// AS: Enemy30.endAnimationCycle
void Enemy30::endAnimationCycle()
{
    if (cycleName == "land") {
        isSquashing = false;
    } else if (cycleName == "unstun") {
        jumpspeed = -10;
        jump = true;
        isJumpUnstunning = true;
    } else if (cycleName == "endFlying") {
        isEndFlying = false;
        isParachuting = false;
        isStunned = false;
    }
}

// AS: Enemy30.removeEnemy — it frees its spawner's slot. The original looks the spawner up among the enemies
// (EnemyManager.getEnemy) and calls unregisterSpawn(type, id) on it.
void Enemy30::removeEnemy()
{
    if (spawnerObjectID != -1) {
        if (Enemy *spawner = gameObj->obf_E->getEnemy(spawnerObjectID, whichScreen)) {
            spawner->unregisterSpawn(type, id);
        }
    }
    Enemy::removeEnemy();
}

// AS: Enemy30.updateObject — parachuting, it sinks 2 px a frame and folds the parachute on landing; on the ground it
// squashes after each landing, then waits 4 frames and hops towards the player.
void Enemy30::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    managers::EffectManager &fx = *gameObj->obf_V;
    characters::CustomerChar &p = *gameObj->playerObj;
    speed = adjustSpeedForSlopes(walkspeed);
    if (isDead) {
        jumpChar(false);
        moveChar(dirx, 0, 0, false, false, false);
        if (dizzyFXID != -1) {
            fx.removeEffect(dizzyFXID);
            dizzyFXID = -1;
        }
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
    const bool wasJumping = jump;
    if (!isParachuting && !isEndFlying) {
        if (!isSquashing && jump) {
            moveChar(dirx, 0, 0, true);
        }
        fallChar();
        if (jump) {
            jumpChar();
        }
    }
    facingDir = dirx;
    walkingDir = dirx;
    if (!isParachuting && !isEndFlying && !jump && !isStunned && !isUnstunning) {
        if (wasJumping) {
            isSquashing = true;
        }
        if (!isSquashing) {
            ++delayTimer;
            if (delayTimer >= delayBeforeJump) {
                dirx = p.x < x ? -1 : 1;
                facingDir = dirx;
                walkingDir = dirx;
                jump = true;
                jumpspeed = jumpstart;
                delayTimer = 0;
                speed = walkspeed;
            }
        }
    }
    if (isParachuting) {
        speed = 2;
        moveChar(0, 1, 0);
        if (!jump) {
            isParachuting = false;
            isEndFlying = true;
            isStunned = true;
        }
    }
    if (isStunned && !isEndFlying && !isParachuting) {
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
        dizzyFXID = fx.addEffect(x, y - 30, "DizzyEffect", facingDir == 1 ? "faceRight" : "faceLeft");
    } else if (!isStunned && dizzyFXID != -1) {
        fx.removeEffect(dizzyFXID);
        dizzyFXID = -1;
    }
}

// AS: Enemy30.checkOnScreen — a column either side, and from 5 rows above the screen to 2 below it.
bool Enemy30::checkOnScreen()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    if (xtile > d.currentXtileLeft - 1 && xtile < d.currentXtileRight + 1) {
        return ytile < d.currentYtileDown + 2 && ytile > d.currentYtileUp - 5;
    }
    return false;
}

} // namespace papaGame::models::enemies
