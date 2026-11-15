#include "game/papaGame/models/enemies/Enemy2.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy2.defineVars
void Enemy2::defineVars()
{
    type = 2;
    sheetname = "enemysheet_brownonion";
    sheetIsBitmap = false;
    height = 15;
    width = 15;
    playerheight = 15;
    spritewidth = 56;
    spriteheight = 66;
    spriteCenterX = 28;
    spriteCenterY = 33;
    spriteTargetX = 32;
    spriteTargetY = 46;
    sheetWidth = 26;
    sheetHeight = 4;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 2;
    walkspeed = 2;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -12;
    flipBlitOffset = 132;
    collRect = {-15, -20, 30, 40};
    gravity = 2;
    normalgravity = 2;
    currentHealth = 1;
    maxHealth = 1;
    facingWalking = true;
    points = 10;
    canBeStunned = true;
    stunDuration = 90;
    enemyCategory = "onion";
}

// AS: Enemy2 constructor — paces between its start tile and targetTileX (or walks freely without one).
void Enemy2::init(double xtile_, double ytile_, double id_, double whichScreen_, double whichTileset_,
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

// AS: Enemy2.setupParams — [facing, rememberKilled, target tile, spawner object, (spawned: chase the player)]
void Enemy2::setupParams(const std::vector<as3::Value> &params)
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
        targetTileX = params[2].asNumber();
    }
    if (params.size() > 3) {
        spawnerObjectID = params[3].asNumber();
    }
    if (params.size() > 4) {
        hasJumped = true;
        noticedPlayer = true;
        alwaysNoticePlayer = true;
    }
}

// AS: Enemy2.stunEnemy
void Enemy2::stunEnemy()
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
    if (!didTagStun) {
        // SKELETON: ChallengeManager.recordTag("enemyStun").
        didTagStun = true;
    }
}

// AS: Enemy2.reactToHurtingPlayer
void Enemy2::reactToHurtingPlayer()
{
    isReactingToHurt = true;
    jumpspeed = -10;
    jump = true;
}

// AS: Enemy2.grabAnimationCycle
const AnimationCycle &Enemy2::grabAnimationCycle()
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
    if (isTurning) {
        return cycleFrames("turnCycleFrames");
    }
    if (isWalking) {
        return cycleFrames("walkCycleFrames");
    }
    return cycleFrames("standCycleFrames");
}

// AS: Enemy2.endAnimationCycle
void Enemy2::endAnimationCycle()
{
    if (cycleName == "turn") {
        isTurning = false;
    } else if (cycleName == "unstun") {
        jumpspeed = -10;
        jump = true;
        isJumpUnstunning = true;
    }
}

// AS: Enemy2.removeEnemy
void Enemy2::removeEnemy()
{
    if (spawnerObjectID != -1) {
        // SKELETON: ObjectManager.getObject(spawnerObjectID, whichScreen).unregisterSpawn() (objects).
    }
    Enemy::removeEnemy();
}

// AS: Enemy2.updateObject
void Enemy2::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    speed = adjustSpeedForSlopes(walkspeed);
    if (isDead) {
        jumpChar(false);
        moveChar(dirx, 0, 0, false, false, false);
        if (dizzyFXID != -1) {
            gameObj->obf_V->removeEffect(dizzyFXID);
            dizzyFXID = -1;
        }
        updateSprite();
        if (ytile > d.currentYtileDown + 1) {
            removeEnemy();
        } else if (isLaunching && (ytile <= d.currentYtileUp - 1 || ytile < 0)) {
            removeEnemy();
        }
    } else if (checkOnScreen() || spawnerObjectID != -1) {
        const double lastDirX = dirx;
        if (!isStunned && !isUnstunning && !isReactingToHurt) {
            if (leftTileX != rightTileX) {
                moveChar(dirx, 0, 0, true, true);
            }
        }
        fallChar();
        if (jump) {
            jumpChar();
        }
        if (!isStunned && !isUnstunning && targetTileX != -1 && leftTileX != rightTileX) {
            if (dirx == 1 && x > rightTileX * d.tileWidth + d.tileWidth / 2) {
                x = rightTileX * d.tileWidth + d.tileWidth / 2;
                xtile = std::floor(x / d.tileWidth);
                dirx = -1;
            } else if (dirx == -1 && x < leftTileX * d.tileWidth + d.tileWidth / 2) {
                x = leftTileX * d.tileWidth + d.tileWidth / 2;
                xtile = std::floor(x / d.tileWidth);
                dirx = 1;
            }
        }
        facingDir = dirx;
        walkingDir = dirx;
        if (dirx != lastDirX) {
            isTurning = true;
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
        updateSprite();
        if (dizzyFXID == -1 && isStunned && cycleFrame == cycleFrames("stunCycleFrames").loopFrame) {
            dizzyFXID = gameObj->obf_V->addEffect(x, y - 30, "DizzyEffect", facingDir == 1 ? "faceRight" : "faceLeft");
        } else if (!isStunned && dizzyFXID != -1) {
            gameObj->obf_V->removeEffect(dizzyFXID);
            dizzyFXID = -1;
        }
    }
    spriteTargetX = facingDir == 1 ? 32 : 24;
}

} // namespace papaGame::models::enemies
