#include "game/papaGame/models/enemies/Enemy21.h"

#include <cmath>

#include "game/as3.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy21.defineVars
void Enemy21::defineVars()
{
    type = 21;
    sheetname = "enemysheet_bowshroom";
    sheetIsBitmap = false;
    height = 15;
    width = 15;
    playerheight = 15;
    spritewidth = 74;
    spriteheight = 80;
    spriteCenterX = 37;
    spriteCenterY = 40;
    spriteTargetX = 37;
    spriteTargetY = 60;
    sheetWidth = 25;
    sheetHeight = 4;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 2;
    walkspeed = 2;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -12;
    flipBlitOffset = 160;
    collRect = {-15, -20, 30, 40};
    gravity = 2;
    normalgravity = 2;
    currentHealth = 1;
    maxHealth = 1;
    facingWalking = true;
    points = 10;
    canBeStunned = true;
    stunDuration = 90;
    bouncingDoesStun = false;
    enemyCategory = "shroom";
}

// AS: Enemy21.setupParams — [facing, rememberKilled, starts up (1) or down, frames up, frames down (0: random),
// arrows per rise, frames before the first shot, shoot only when the player is in sight, turn to face the player].
void Enemy21::setupParams(const std::vector<as3::Value> &params)
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
        isDown = params[2].asNumber() != 1;
    }
    if (params.size() > 3) {
        upTimeMax = params[3].asNumber();
    }
    if (params.size() > 4) {
        downTimeMax = params[4].asNumber();
    }
    if (params.size() > 5) {
        maxArrows = params[5].asNumber();
    }
    if (params.size() > 6) {
        shootDelay = params[6].asNumber();
    }
    if (params.size() > 7) {
        onlyShootIfSeesPlayer = params[7].asNumber() == 1;
    }
    if (params.size() > 8) {
        shouldTurnToFace = params[8].asNumber() == 1;
    }
}

// AS: Enemy21.stunEnemy — a stomp makes it bounce (again) and loses its arrow.
void Enemy21::stunEnemy()
{
    if (!isStunned) {
        isStunned = true;
    } else {
        cycleFrame = 0;
        animationFrame = cycleFrames(isDown ? "bouncedownCycleFrames" : "bounceCycleFrames").frameAt(0);
    }
    hasArrow = false;
    isUnstunning = false;
    stunStartTime = 0;
}

// AS: Enemy21.grabAnimationCycle
const AnimationCycle &Enemy21::grabAnimationCycle()
{
    if (isDead) {
        return cycleFrames("dieCycleFrames");
    }
    if (isStunned) {
        return cycleFrames(isDown ? "bouncedownCycleFrames" : "bounceCycleFrames");
    }
    if (isShooting) {
        return cycleFrames("shootCycleFrames");
    }
    if (!hasArrow && !isDown) {
        return cycleFrames("reloadCycleFrames");
    }
    if (isPulling) {
        return cycleFrames("pullbackCycleFrames");
    }
    if (isTurning) {
        return cycleFrames("turnCycleFrames");
    }
    if (isDropping) {
        return cycleFrames("duckCycleFrames");
    }
    if (isRising) {
        return cycleFrames("unduckCycleFrames");
    }
    if (isDown) {
        return cycleFrames("downCycleFrames");
    }
    return cycleFrames("standCycleFrames");
}

// AS: Enemy21.endAnimationCycle — the arrow leaves at the end of the pull.
void Enemy21::endAnimationCycle()
{
    if (cycleName == "turn") {
        isTurning = false;
    } else if (cycleName == "bounce" || cycleName == "bouncedown") {
        isStunned = false;
    } else if (cycleName == "duck") {
        isDropping = false;
        isDown = true;
    } else if (cycleName == "unduck") {
        isRising = false;
        isDown = false;
    } else if (cycleName == "pullback") {
        isPulling = false;
        fireArrow();
    } else if (cycleName == "shoot") {
        isShooting = false;
    } else if (cycleName == "reload") {
        hasArrow = true;
    }
}

// AS: Enemy21.fireArrow
void Enemy21::fireArrow()
{
    isShooting = true;
    hasArrow = false;
    gameObj->obf_4l->addBullet(x + 10 * facingDir, y, facingDir, 0, "ShroomArrow", "", true, 1, true);
    gameObj->obf_7C->playSound("battle_bow1.wav");
    ++shotArrows;
}

// AS: Enemy21.updateObject — up, it shoots its arrows (after shootDelay frames) and then ducks after upTimeMax
// frames; down, it rises after downTimeMax (a 2 % chance per frame when the time is 0) with a new arrow.
void Enemy21::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    const characters::CustomerChar &p = *gameObj->playerObj;
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
    collRect = isDown ? downRect : upRect;
    const double oldDir = dirx;
    if (!isDown && !isRising && !isDropping && !isStunned && !isTurning && !isShooting && !isPulling && hasArrow) {
        if (shouldTurnToFace) {
            dirx = p.x < x ? -1 : 1;
        }
        facingDir = dirx;
        walkingDir = dirx;
        if (dirx != oldDir) {
            isTurning = true;
        }
    }
    auto play = [&](const char *sound) {
        if (isReallyOnScreen()) {
            gameObj->obf_7C->playSound(sound, false, getProximityVolume(), getProximityPan());
        }
    };
    if (isDown && !isDropping && !isRising && !isStunned && !isShooting && !isPulling) {
        ++stateTimer;
        if (downTimeMax == 0) {
            if (as3::random() > 0.98) {
                isRising = true;
                stateTimer = 0;
                hasArrow = true;
                play("mushroom_rise.wav");
            }
        } else if (stateTimer >= downTimeMax) {
            isRising = true;
            stateTimer = 0;
            hasArrow = true;
            play("mushroom_rise.wav");
        }
    } else if (!isDown && !isDropping && !isRising && !isStunned && !isShooting && !isPulling && hasArrow) {
        ++stateTimer;
        if (shotArrows < maxArrows && stateTimer >= shootDelay && hasArrow) {
            if (!onlyShootIfSeesPlayer) {
                isPulling = true;
            } else {
                const double dx = x - p.x;
                const double dy = y - p.y;
                if (std::abs(dy) < 90 &&
                    ((facingDir == -1 && dx < 300) || (facingDir == 1 && dx > -300 && dx < 0))) {
                    isPulling = true;
                }
            }
        }
        if (!isShooting && !isPulling) {
            if (upTimeMax == 0) {
                if (as3::random() > 0.98) {
                    isDropping = true;
                    shotArrows = 0;
                    stateTimer = 0;
                    play("mushroom_drop.wav");
                }
            } else if (stateTimer >= upTimeMax) {
                isDropping = true;
                shotArrows = 0;
                stateTimer = 0;
                play("mushroom_drop.wav");
            }
        }
    }
    updateSprite();
}

// AS: Enemy21.getHit — not while it is down.
bool Enemy21::getHit(double damage, double dir, bool unused, const std::string &weapon)
{
    if (isDown) {
        return false;
    }
    return Enemy::getHit(damage, dir, unused, weapon);
}

} // namespace papaGame::models::enemies
