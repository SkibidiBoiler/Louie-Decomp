#include "game/papaGame/models/enemies/Enemy20.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy20.defineVars
void Enemy20::defineVars()
{
    type = 20;
    sheetname = "enemysheet_thornshroom";
    sheetIsBitmap = false;
    height = 15;
    width = 15;
    playerheight = 15;
    spritewidth = 86;
    spriteheight = 84;
    spriteCenterX = 43;
    spriteCenterY = 42;
    spriteTargetX = 43;
    spriteTargetY = 68;
    sheetWidth = 22;
    sheetHeight = 2;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 2;
    walkspeed = 2;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -12;
    flipBlitOffset = 84;
    collRect = {-15, -20, 30, 40};
    gravity = 2;
    normalgravity = 2;
    currentHealth = 1;
    maxHealth = 1;
    facingWalking = true;
    points = 10;
    canBeStunned = false;
    stunDuration = 90;
    bouncingDoesStun = false;
    enemyCategory = "shroom";
}

// AS: Enemy20.setupParams — [facing, rememberKilled, starts up (1) or down, frames up, frames down (0: random)].
void Enemy20::setupParams(const std::vector<as3::Value> &params)
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

// AS: Enemy20.grabAnimationCycle
const AnimationCycle &Enemy20::grabAnimationCycle()
{
    if (isDead) {
        return cycleFrames("dieCycleFrames");
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

// AS: Enemy20.endAnimationCycle
void Enemy20::endAnimationCycle()
{
    if (cycleName == "turn") {
        isTurning = false;
    } else if (cycleName == "duck") {
        isDropping = false;
        isDown = true;
    } else if (cycleName == "unduck") {
        isRising = false;
        isDown = false;
    }
}

// AS: Enemy20.updateObject — up, it turns to face the player; it ducks after upTimeMax frames and rises after
// downTimeMax (a 2 % chance per frame when the time is 0).
void Enemy20::updateObject()
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
                play("thornshroom_rise.wav");
            }
        } else if (stateTimer >= downTimeMax) {
            isRising = true;
            stateTimer = 0;
            play("thornshroom_rise.wav");
        }
    } else if (!isDown && !isDropping && !isRising && !isStunned) {
        ++stateTimer;
        if (upTimeMax == 0) {
            if (as3::random() > 0.98) {
                isDropping = true;
                stateTimer = 0;
                play("thornshroom_drop.wav");
            }
        } else if (stateTimer >= upTimeMax) {
            isDropping = true;
            stateTimer = 0;
            play("thornshroom_drop.wav");
        }
    }
    updateSprite();
}

// AS: Enemy20.getHit — not while it is down.
bool Enemy20::getHit(double damage, double dir, bool unused, const std::string &weapon)
{
    if (isDown) {
        return false;
    }
    return Enemy::getHit(damage, dir, unused, weapon);
}

} // namespace papaGame::models::enemies
