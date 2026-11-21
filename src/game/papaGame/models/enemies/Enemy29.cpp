#include "game/papaGame/models/enemies/Enemy29.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy29.Enemy29 — it starts on its ladder.
void Enemy29::init(double xtile_, double ytile_, double id_, double whichScreen_, double whichTileset_,
                   double screenObjID_, const std::vector<as3::Value> &extraParams)
{
    Enemy::init(xtile_, ytile_, id_, whichScreen_, whichTileset_, screenObjID_, extraParams);
    jump = false;
    isClimbingLadder = true;
}

// AS: Enemy29.defineVars
void Enemy29::defineVars()
{
    type = 29;
    sheetname = "enemysheet_laddish";
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
    sheetWidth = 21;
    sheetHeight = 2;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 2;
    walkspeed = 2;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -12;
    flipBlitOffset = 0;
    collRect = {-15, -25, 30, 30};
    gravity = 2;
    normalgravity = 2;
    currentHealth = 1;
    maxHealth = 1;
    facingWalking = true;
    points = 10;
    canBeStunned = true;
    stunDuration = 90;
    foreground = true;
    rememberKilled = false;
    isClimbingLadder = true;
    enemyCategory = "radish";
}

// AS: Enemy29.setupParams — [climbing direction] (1 down, -1 up; down when missing).
void Enemy29::setupParams(const std::vector<as3::Value> &params)
{
    climbDir = params.size() > 0 ? params[0].asNumber() : 1;
}

// AS: Enemy29.stunEnemy — a stomp knocks it off the ladder.
void Enemy29::stunEnemy()
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
    isClimbingLadder = false;
    jump = true;
    jumpspeed = 0;
    if (!didTagStun) {
        gameObj->obf_94->recordTag("enemyStun");
        didTagStun = true;
    }
}

// AS: Enemy29.grabAnimationCycle
const AnimationCycle &Enemy29::grabAnimationCycle()
{
    if (isDead) {
        return cycleFrames("dieCycleFrames");
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
    return cycleFrames("climbCycleFrames");
}

// AS: Enemy29.endAnimationCycle
void Enemy29::endAnimationCycle()
{
    if (cycleName == "unstun") {
        jumpspeed = -10;
        jump = true;
        isJumpUnstunning = true;
    }
}

// AS: Enemy29.updateObject — on the ladder it climbs and turns where the ladder tiles end; off it, it falls; once it
// has recovered on the ground it climbs again, upwards.
void Enemy29::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    managers::EffectManager &fx = *gameObj->obf_V;
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
    if (isClimbingLadder) {
        moveChar(0, climbDir, 0);
        const managers::MapManager &map = *gameObj->obf_4i;
        if (climbDir == -1) {
            if (map.getTileProperty(xtile, upY, "ladder") != 1) {
                climbDir = 1;
            }
        } else if (climbDir == 1) {
            if (map.getTileProperty(xtile, downY, "ladder") != 1) {
                climbDir = -1;
            }
        }
    } else {
        fallChar();
        if (jump) {
            jumpChar();
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
    if (!isStunned && !isUnstunning && !jump && !isClimbingLadder) {
        isClimbingLadder = true;
        climbDir = -1;
    }
    updateSprite();
    if (dizzyFXID == -1 && isStunned && cycleFrame == cycleFrames("stunCycleFrames").loopFrame) {
        dizzyFXID = fx.addEffect(x, y - 30, "DizzyEffect", "faceLeft");
    } else if (!isStunned && dizzyFXID != -1) {
        fx.removeEffect(dizzyFXID);
        dizzyFXID = -1;
    }
}

// AS: Enemy29.checkOnScreen — only the columns count.
bool Enemy29::checkOnScreen()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 2 && xtile < d.currentXtileRight + 2;
}

} // namespace papaGame::models::enemies
