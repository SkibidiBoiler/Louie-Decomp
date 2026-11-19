#include "game/papaGame/models/enemies/Enemy8.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy8.defineVars
void Enemy8::defineVars()
{
    type = 8;
    sheetname = "enemysheet_runttomato";
    sheetIsBitmap = false;
    height = 15;
    width = 15;
    playerheight = 15;
    spritewidth = 36;
    spriteheight = 42;
    spriteCenterX = 18;
    spriteCenterY = 21;
    spriteTargetX = 18;
    spriteTargetY = 25;
    sheetWidth = 19;
    sheetHeight = 2;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 4;
    walkspeed = 4;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -14;
    flipBlitOffset = 42;
    collRect = {-15, -12, 30, 24};
    gravity = 1;
    normalgravity = 1;
    currentHealth = 1;
    maxHealth = 1;
    facingWalking = true;
    points = 10;
    canBeStunned = true;
    enemyCategory = "tomato";
}

// AS: Enemy8.setupParams — [facing (reversed: it turns before its first jump), rememberKilled, spawner, (spawned)]
void Enemy8::setupParams(const std::vector<as3::Value> &params)
{
    if (params.size() > 0) {
        facingDir = params[0].asNumber() * -1;
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

// AS: Enemy8.stunEnemy — a stun is a hit, away from the player.
void Enemy8::stunEnemy()
{
    double dir = 1;
    if (gameObj->playerObj->x > x) {
        dir = -1;
    }
    getHit(1, dir, false);
}

// AS: Enemy8.grabAnimationCycle
const AnimationCycle &Enemy8::grabAnimationCycle()
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
    if (isSquashing) {
        return cycleFrames("landCycleFrames");
    }
    return cycleFrames("standCycleFrames");
}

// AS: Enemy8.endAnimationCycle
void Enemy8::endAnimationCycle()
{
    if (cycleName == "land") {
        isSquashing = false;
    }
}

// AS: Enemy8.removeEnemy
void Enemy8::removeEnemy()
{
    if (spawnerObjectID != -1) {
        if (GameObject *spawner = gameObj->obf_7u->getObject(spawnerObjectID, whichScreen)) {
            spawner->unregisterSpawn();
        }
    }
    Enemy::removeEnemy();
}

// AS: Enemy8.updateObject — it moves only in the air; on the ground it waits, turns round and jumps.
void Enemy8::updateObject()
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
        const bool wasJumping = jump;
        if (!isSquashing && jump) {
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
                ++delayTimer;
                if (delayTimer >= delayBeforeJump) {
                    dirx *= -1;
                    facingDir = dirx;
                    walkingDir = dirx;
                    jump = true;
                    jumpspeed = jumpstart;
                    delayTimer = 0;
                    if (isReallyOnScreen()) {
                        gameObj->obf_7C->playSound("jump_bacobite.wav", false, getProximityVolume(), getProximityPan());
                    }
                }
            }
        }
        updateSprite();
    }
}

// AS: Enemy8.killMe
void Enemy8::killMe(double dir)
{
    Enemy::killMe(dir);
    if (gameObj->playerObj && gameObj->playerObj->playerData() && gameObj->playerObj->playerData()->customerName == "Greg") {
        gameObj->obf_94->recordTag("gregtomato");
    }
}

} // namespace papaGame::models::enemies
