#include "game/papaGame/models/enemies/Enemy19.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy19.defineVars
void Enemy19::defineVars()
{
    type = 19;
    sheetname = "enemysheet_blueshroom";
    sheetIsBitmap = false;
    height = 15;
    width = 15;
    playerheight = 15;
    spritewidth = 52;
    spriteheight = 68;
    spriteCenterX = 26;
    spriteCenterY = 34;
    spriteTargetX = 26;
    spriteTargetY = 52;
    sheetWidth = 30;
    sheetHeight = 2;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 2;
    walkspeed = 2;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -12;
    flipBlitOffset = 68;
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

// AS: Enemy19.setupParams — [facing, rememberKilled, starts up (1) or down, frames up, frames down (0: random)].
void Enemy19::setupParams(const std::vector<as3::Value> &params)
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
}

// AS: Enemy19.stunEnemy — a stomp makes it bounce (again).
void Enemy19::stunEnemy()
{
    if (!isStunned) {
        isStunned = true;
    } else {
        cycleFrame = 0;
        animationFrame = cycleFrames(isDown ? "bouncedownCycleFrames" : "bounceCycleFrames").frameAt(0);
    }
    isUnstunning = false;
    stunStartTime = 0;
}

// AS: Enemy19.grabAnimationCycle
const AnimationCycle &Enemy19::grabAnimationCycle()
{
    if (isDead) {
        return cycleFrames("dieCycleFrames");
    }
    if (isStunned) {
        return cycleFrames(isDown ? "bouncedownCycleFrames" : "bounceCycleFrames");
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

// AS: Enemy19.endAnimationCycle
void Enemy19::endAnimationCycle()
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
    }
}

// AS: Enemy19.updateObject — up, it turns to face the player; it ducks after upTimeMax frames and rises after
// downTimeMax (a 2 % chance per frame when the time is 0).
void Enemy19::updateObject()
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
    if (!isDown && !isRising && !isDropping && !isStunned && !isTurning) {
        dirx = p.x < x ? -1 : 1;
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
    if (isDown && !isDropping && !isRising && !isStunned) {
        ++stateTimer;
        if (downTimeMax == 0) {
            if (as3::random() > 0.98) {
                isRising = true;
                stateTimer = 0;
                play("mushroom_rise.wav");
            }
        } else if (stateTimer >= downTimeMax) {
            isRising = true;
            stateTimer = 0;
            play("mushroom_rise.wav");
        }
    } else if (!isDown && !isDropping && !isRising && !isStunned) {
        ++stateTimer;
        if (upTimeMax == 0) {
            if (as3::random() > 0.98) {
                isDropping = true;
                stateTimer = 0;
                play("mushroom_drop.wav");
            }
        } else if (stateTimer >= upTimeMax) {
            isDropping = true;
            stateTimer = 0;
            play("mushroom_drop.wav");
        }
    }
    updateSprite();
}

// AS: Enemy19.getHit — not while it is down.
bool Enemy19::getHit(double damage, double dir, bool unused, const std::string &weapon)
{
    if (isDown) {
        return false;
    }
    return Enemy::getHit(damage, dir, unused, weapon);
}

} // namespace papaGame::models::enemies
