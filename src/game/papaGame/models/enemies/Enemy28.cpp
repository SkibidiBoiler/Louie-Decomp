#include "game/papaGame/models/enemies/Enemy28.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy28.defineVars
void Enemy28::defineVars()
{
    type = 28;
    sheetname = "enemysheet_radish";
    sheetIsBitmap = false;
    height = 15;
    width = 15;
    playerheight = 15;
    spritewidth = 64;
    spriteheight = 74;
    spriteCenterX = 32;
    spriteCenterY = 37;
    spriteTargetX = 32;
    spriteTargetY = 54;
    sheetWidth = 24;
    sheetHeight = 4;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 2;
    walkspeed = 2;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -12;
    flipBlitOffset = 148;
    collRect = {-15, -20, 30, 40};
    gravity = 2;
    normalgravity = 2;
    currentHealth = 1;
    maxHealth = 1;
    facingWalking = true;
    points = 10;
    canBeStunned = true;
    stunDuration = 90;
    enemyCategory = "radish";
}

// AS: Enemy28.setupParams — [facing, rememberKilled, planted].
void Enemy28::setupParams(const std::vector<as3::Value> &params)
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
        isPlanted = params[2].asNumber() == 1;
        collRect = isPlanted ? plantedRect : regularRect;
    }
}

// AS: Enemy28.stunEnemy — not while popping out.
void Enemy28::stunEnemy()
{
    if (isPoppingOut) {
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
}

// AS: Enemy28.reactToHurtingPlayer — it hops back away from the player.
void Enemy28::reactToHurtingPlayer()
{
    isReactingToHurt = true;
    jumpspeed = -10;
    jump = true;
    dirx = gameObj->playerObj->x < x ? 1 : -1;
}

// AS: Enemy28.grabAnimationCycle
const AnimationCycle &Enemy28::grabAnimationCycle()
{
    if (isDead) {
        return cycleFrames("dieCycleFrames");
    }
    if (isPoppingOut) {
        return cycleFrames("popoutCycleFrames");
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
    if (isUnstunning) {
        return cycleFrames("unstunCycleFrames");
    }
    if (isTurning) {
        return cycleFrames("turnCycleFrames");
    }
    if (isWalking) {
        return cycleFrames("walkCycleFrames");
    }
    if (isPlanted) {
        return cycleFrames("plantedCycleFrames");
    }
    return cycleFrames("walkCycleFrames");
}

// AS: Enemy28.endAnimationCycle — out of the ground it starts walking.
void Enemy28::endAnimationCycle()
{
    if (cycleName == "turn") {
        isTurning = false;
    } else if (cycleName == "popout") {
        isPoppingOut = false;
        isPlanted = false;
        isWalking = true;
        isStunned = false;
    } else if (cycleName == "unstun") {
        jumpspeed = -10;
        jump = true;
        isJumpUnstunning = true;
    }
}

// AS: Enemy28.updateObject — planted, it pops out when the player is within 128 px across and 64 px up or down (not
// more than 30 px below it); out, it turns back towards a player who gets 64 px behind it.
void Enemy28::updateObject()
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
    const double oldDir = dirx;
    if (!isPlanted && !isPoppingOut && !isStunned && !isUnstunning && !isReactingToHurt) {
        moveChar(dirx, 0, 0, true, false);
    }
    fallChar();
    if (jump) {
        jumpChar();
    }
    if (isPlanted && !isPoppingOut) {
        const double dy = std::abs(p.y - y);
        const double dx = std::abs(p.x - x);
        if (dy <= 64 && p.y <= y + 30 && dx <= 128) {
            isPoppingOut = true;
            isStunned = true;
            if (isReallyOnScreen()) {
                gameObj->obf_7C->playSound("radish_popout.wav", false, getProximityVolume(), getProximityPan());
            }
        }
    } else if (!isPlanted && !isPoppingOut) {
        if (p.canGetHit() && !isStunned) {
            if (dirx == 1 && p.x < x - 64) {
                dirx = -1;
            } else if (dirx == -1 && p.x > x + 64) {
                dirx = 1;
            }
        }
    } else if (isPoppingOut) {
        if (animationFrame == 3) {
            fx.addEffect(x, y, "RadishDirtEffect", "piece1");
            fx.addEffect(x - 3, y - 1, "RadishDirtEffect", "piece2");
            fx.addEffect(x + 3, y - 1, "RadishDirtEffect", "piece2");
            fx.addEffect(x - 4, y, "RadishDirtEffect", "piece3");
            fx.addEffect(x + 4, y, "RadishDirtEffect", "piece3");
            fx.addEffect(x, y, "RadishDirtEffect", "piece3");
            fx.addEffect(x, y, "RadishDirtEffect", "piece3");
        } else if (animationFrame == 5) {
            collRect = regularRect;
        }
    }
    facingDir = dirx;
    walkingDir = dirx;
    if (dirx != oldDir) {
        isTurning = true;
    }
    if (!isPoppingOut && isStunned) {
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
    if (dizzyFXID == -1 && isStunned && !isPoppingOut && cycleFrame == cycleFrames("stunCycleFrames").loopFrame) {
        dizzyFXID = fx.addEffect(x, y - 30, "DizzyEffect", facingDir == 1 ? "faceRight" : "faceLeft");
    } else if (!isStunned && dizzyFXID != -1) {
        fx.removeEffect(dizzyFXID);
        dizzyFXID = -1;
    }
}

} // namespace papaGame::models::enemies
