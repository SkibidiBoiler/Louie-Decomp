#include "game/papaGame/models/enemies/Enemy12.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy12.defineVars
void Enemy12::defineVars()
{
    type = 12;
    sheetname = "enemysheet_slider";
    sheetIsBitmap = false;
    height = 15;
    width = 15;
    playerheight = 15;
    spritewidth = 46;
    spriteheight = 46;
    spriteCenterX = 23;
    spriteCenterY = 23;
    spriteTargetX = 23;
    spriteTargetY = 30;
    sheetWidth = 12;
    sheetHeight = 2;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 4;
    walkspeed = 4;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -14;
    flipBlitOffset = 46;
    collRect = {-15, -12, 30, 24};
    gravity = 1;
    normalgravity = 1;
    currentHealth = 1;
    maxHealth = 1;
    facingWalking = true;
    points = 10;
    canBeStunned = true;
    enemyCategory = "slider";
}

// AS: Enemy12.setupParams — [facing (reversed), rememberKilled, spawner object, (spawned)]
void Enemy12::setupParams(const std::vector<as3::Value> &params)
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

// AS: Enemy12.stunEnemy — a stomp hurts it, pushing it away from the player.
void Enemy12::stunEnemy()
{
    double dir = 1;
    if (gameObj->playerObj->x > x) {
        dir = -1;
    }
    getHit(1, dir, false);
}

// AS: Enemy12.grabAnimationCycle
const AnimationCycle &Enemy12::grabAnimationCycle()
{
    if (isDead) {
        return cycleFrames("dieCycleFrames");
    }
    if (isWindingUp) {
        return cycleFrames("windupCycleFrames");
    }
    if (isWalking) {
        return cycleFrames("slideCycleFrames");
    }
    return cycleFrames("standCycleFrames");
}

// AS: Enemy12.endAnimationCycle — the wind-up ends in a slide.
void Enemy12::endAnimationCycle()
{
    if (cycleName == "windup") {
        isWindingUp = false;
        startSliding();
    }
}

// AS: Enemy12.removeEnemy
void Enemy12::removeEnemy()
{
    if (spawnerObjectID != -1) {
        if (GameObject *spawner = gameObj->obf_7u->getObject(spawnerObjectID, whichScreen)) {
            spawner->unregisterSpawn();
        }
    }
    Enemy::removeEnemy();
}

// AS: Enemy12.startSliding
void Enemy12::startSliding()
{
    walkspeed = startingSlideSpeed;
    isWalking = true;
    isWindingUp = false;
    delayTimer = 0;
    if (isReallyOnScreen()) {
        gameObj->obf_7C->playSound("slide_slider.wav", false, getProximityVolume(), getProximityPan());
    }
}

// AS: Enemy12.updateObject — the slide loses a unit of speed every slideDecay frames; once stopped it waits, then
// turns and winds up again.
void Enemy12::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    speed = adjustSpeedForSlopes(walkspeed);
    if (isDead) {
        walkspeed = 3;
        jumpChar(false);
        moveChar(dirx, 0, 0, false, false, false);
        updateSprite();
        if (ytile > d.currentYtileDown + 1) {
            removeEnemy();
        } else if (isLaunching && (ytile <= d.currentYtileUp - 1 || ytile < 0)) {
            removeEnemy();
        }
    } else if (checkOnScreen() || spawnerObjectID != -1) {
        if (isWalking) {
            if (walkspeed > 0) {
                if (std::fmod(gameObj->obf_8o->gameplayTimer, slideDecay) == 0) {
                    --walkspeed;
                }
            } else {
                isWalking = false;
            }
        }
        if (isWalking && !isWindingUp) {
            moveChar(dirx, 0, 0, true);
        }
        fallChar();
        if (jump) {
            jumpChar();
        }
        facingDir = dirx;
        walkingDir = dirx;
        if (!isWalking) {
            if (!isWindingUp) {
                ++delayTimer;
                if (delayTimer >= delayBeforeSlide) {
                    dirx *= -1;
                    facingDir = dirx;
                    walkingDir = dirx;
                    isWindingUp = true;
                }
            }
        }
        updateSprite();
    }
}

// AS: Enemy12.killMe
void Enemy12::killMe(double dir)
{
    Enemy::killMe(dir);
    if (gameObj->playerObj && gameObj->playerObj->playerData() && gameObj->playerObj->playerData()->customerName == "Rita") {
        gameObj->obf_94->recordTag("ritaslider");
    }
}

} // namespace papaGame::models::enemies
