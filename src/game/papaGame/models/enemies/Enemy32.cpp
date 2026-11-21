#include "game/papaGame/models/enemies/Enemy32.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy32.Enemy32 — its walk runs between its column and the target column; without a target it walks freely
// (columns -1 to 600); a target equal to its column makes it stand.
void Enemy32::init(double xtile_, double ytile_, double id_, double whichScreen_, double whichTileset_,
                   double screenObjID_, const std::vector<as3::Value> &extraParams)
{
    Enemy::init(xtile_, ytile_, id_, whichScreen_, whichTileset_, screenObjID_, extraParams);
    if (targetTileX != -1) {
        if (xtile < targetTileX) {
            leftTileX = xtile;
            rightTileX = targetTileX;
        } else if (xtile > targetTileX) {
            leftTileX = targetTileX;
            rightTileX = xtile;
        } else if (xtile == targetTileX) {
            leftTileX = xtile;
            rightTileX = xtile;
        }
    } else {
        leftTileX = -1;
        rightTileX = 600;
    }
}

// AS: Enemy32.defineVars
void Enemy32::defineVars()
{
    type = 32;
    sheetname = "enemysheet_jellyback";
    sheetIsBitmap = false;
    height = 15;
    width = 15;
    playerheight = 15;
    spritewidth = 70;
    spriteheight = 60;
    spriteCenterX = 35;
    spriteCenterY = 30;
    spriteTargetX = 35;
    spriteTargetY = 44;
    sheetWidth = 26;
    sheetHeight = 4;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 2;
    walkspeed = 2;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -12;
    flipBlitOffset = 120;
    collRect = {-15, -20, 30, 40};
    gravity = 2;
    normalgravity = 2;
    currentHealth = 1;
    maxHealth = 1;
    facingWalking = true;
    points = 10;
    canBeStunned = true;
    stunDuration = 90;
    canDealDamage = false;
    stunBounceGravity = 1;
    stunBounceJumpSpeed = -24;
    shoveAmount = 3;
}

// AS: Enemy32.setupParams — [facing, target column].
void Enemy32::setupParams(const std::vector<as3::Value> &params)
{
    if (params.size() > 0) {
        facingDir = params[0].asNumber();
        walkingDir = facingDir;
        dirx = walkingDir;
    }
    if (params.size() > 1) {
        targetTileX = params[1].asNumber();
    }
}

// AS: Enemy32.getHit — a hit only makes it bounce.
bool Enemy32::getHit(double, double, bool, const std::string &)
{
    isHit = false;
    if (!isStunned) {
        isStunned = true;
    } else {
        cycleFrame = 0;
        animationFrame = cycleFrames("bounceCycleFrames").frameAt(0);
    }
    return false;
}

// AS: Enemy32.stunEnemy — the first stomp also counts it as met.
void Enemy32::stunEnemy()
{
    if (!isStunned) {
        isStunned = true;
    } else {
        cycleFrame = 0;
        animationFrame = cycleFrames("bounceCycleFrames").frameAt(0);
    }
    isUnstunning = false;
    isJumpUnstunning = false;
    stunStartTime = 0;
    if (!didTrack) {
        saveEnemyKill();
        didTrack = true;
    }
}

// AS: Enemy32.grabAnimationCycle
const AnimationCycle &Enemy32::grabAnimationCycle()
{
    if (isStunned) {
        return cycleFrames("bounceCycleFrames");
    }
    if (leftTileX == rightTileX) {
        return cycleFrames("standCycleFrames");
    }
    if (isTurning) {
        return cycleFrames("turnCycleFrames");
    }
    return cycleFrames("walkCycleFrames");
}

// AS: Enemy32.endAnimationCycle
void Enemy32::endAnimationCycle()
{
    if (cycleName == "turn") {
        isTurning = false;
    } else if (cycleName == "bounce") {
        isStunned = false;
    }
}

// AS: Enemy32.updateObject — walks (not while bouncing), turning at the ends of its run.
void Enemy32::updateObject()
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
        return;
    }
    if (!checkOnScreen() && spawnerObjectID == -1) {
        return;
    }
    const double oldDir = dirx;
    if (!isStunned && !isUnstunning && !isReactingToHurt && leftTileX != rightTileX) {
        moveChar(dirx, 0, 0, true, true);
    }
    fallChar();
    if (jump) {
        jumpChar();
    }
    if (!isStunned && !isUnstunning && targetTileX != -1 && leftTileX != rightTileX) {
        const double rightEndX = rightTileX * d.tileWidth + d.tileWidth / 2;
        const double leftEndX = leftTileX * d.tileWidth + d.tileWidth / 2;
        if (dirx == 1 && x > rightEndX) {
            x = rightEndX;
            xtile = std::floor(x / d.tileWidth);
            dirx = -1;
        } else if (dirx == -1 && x < leftEndX) {
            x = leftEndX;
            xtile = std::floor(x / d.tileWidth);
            dirx = 1;
        }
    }
    facingDir = dirx;
    walkingDir = dirx;
    if (dirx != oldDir) {
        isTurning = true;
    }
    updateSprite();
}

} // namespace papaGame::models::enemies
