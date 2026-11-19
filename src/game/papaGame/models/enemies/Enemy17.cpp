#include "game/papaGame/models/enemies/Enemy17.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy17.Enemy17 — it starts hanging if there is a ceiling (or a one-way tile) to hang from.
void Enemy17::init(double xtile_, double ytile_, double id_, double whichScreen_, double whichTileset_,
                   double screenObjID_, const std::vector<as3::Value> &extraParams)
{
    Enemy::init(xtile_, ytile_, id_, whichScreen_, whichTileset_, screenObjID_, extraParams);
    findStartupSurface();
}

// AS: Enemy17.defineVars
void Enemy17::defineVars()
{
    type = 17;
    sheetname = "enemysheet_dillworm";
    sheetIsBitmap = false;
    height = 15;
    width = 15;
    playerheight = 15;
    spritewidth = 90;
    spriteheight = 56;
    spriteCenterX = 45;
    spriteCenterY = 28;
    spriteTargetX = 45;
    spriteTargetY = 30;
    sheetWidth = 28;
    sheetHeight = 2;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 2;
    walkspeed = 2;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -12;
    flipBlitOffset = 56;
    collRect = {-15, -20, 30, 40};
    gravity = 2;
    normalgravity = 2;
    currentHealth = 1;
    maxHealth = 1;
    facingWalking = true;
    points = 10;
    canBeStunned = true;
    stunDuration = 90;
    enemyCategory = "dill";
}

// AS: Enemy17.setupParams — [facing, rememberKilled, spawner object, (spawned)]
void Enemy17::setupParams(const std::vector<as3::Value> &params)
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
        spawnerObjectID = params[2].asNumber();
    }
    if (params.size() > 3) {
        hasJumped = true;
        noticedPlayer = true;
        alwaysNoticePlayer = true;
    }
}

// AS: Enemy17.stunEnemy — not while it hangs or drops.
void Enemy17::stunEnemy()
{
    if (!isDropping && !isHanging) {
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
}

// AS: Enemy17.reactToHurtingPlayer
void Enemy17::reactToHurtingPlayer()
{
    if (!isHanging && !isDropping) {
        isReactingToHurt = true;
        jumpspeed = -10;
        jump = true;
    }
}

// AS: Enemy17.grabAnimationCycle
const AnimationCycle &Enemy17::grabAnimationCycle()
{
    if (isDead) {
        return cycleFrames("dieCycleFrames");
    }
    if (isStunned) {
        return cycleFrames("stunCycleFrames");
    }
    if (isDropping) {
        return cycleFrames("fallCycleFrames");
    }
    if (jump) {
        return cycleFrames("crawlCycleFrames");
    }
    if (isSquashing) {
        return cycleFrames("landCycleFrames");
    }
    if (isUnstunning) {
        return cycleFrames("unstunCycleFrames");
    }
    if (isTurning) {
        return cycleFrames("turnCycleFrames");
    }
    if (isWalking) {
        return cycleFrames("crawlCycleFrames");
    }
    if (isNoticing) {
        return cycleFrames("noticeCycleFrames");
    }
    if (isHanging) {
        return cycleFrames("hangCycleFrames");
    }
    return cycleFrames("crawlCycleFrames");
}

// AS: Enemy17.endAnimationCycle — after noticing the player it lets go.
void Enemy17::endAnimationCycle()
{
    if (cycleName == "turn") {
        isTurning = false;
    } else if (cycleName == "unstun") {
        jumpspeed = -10;
        jump = true;
        isJumpUnstunning = true;
    } else if (cycleName == "land") {
        isSquashing = false;
    } else if (cycleName == "notice") {
        isNoticing = false;
        isDropping = true;
        isHanging = false;
        jump = true;
        jumpspeed = 0;
        gameObj->obf_7C->playSound("dillworm_drop.wav", false, 1, getProximityPan());
    }
}

// AS: Enemy17.removeEnemy
void Enemy17::removeEnemy()
{
    if (spawnerObjectID != -1) {
        if (GameObject *spawner = gameObj->obf_7u->getObject(spawnerObjectID, whichScreen)) {
            spawner->unregisterSpawn();
        }
    }
    Enemy::removeEnemy();
}

// AS: Enemy17.updateObject — hanging, it watches for the player below (within 32 px); down, it crawls, turning to
// follow the player once 30 px past it.
void Enemy17::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    managers::EffectManager &fx = *gameObj->obf_V;
    const models::characters::CustomerChar &player = *gameObj->playerObj;
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
    } else if (checkOnScreen() || spawnerObjectID != -1) {
        const double lastDirX = dirx;
        if (!isHanging && !isDropping && !isSquashing && !isStunned && !isUnstunning && !isReactingToHurt) {
            moveChar(dirx, 0, 0);
        }
        if (!isHanging) {
            fallChar();
            if (jump) {
                jumpChar();
            }
            if (!isDropping && !isSquashing && !isStunned && !isUnstunning) {
                if (dirx == 1 && player.x + 30 < x) {
                    dirx = -1;
                } else if (dirx == -1 && player.x - 30 > x) {
                    dirx = 1;
                }
            }
            facingDir = dirx;
            walkingDir = dirx;
            if (dirx != lastDirX) {
                isTurning = true;
            }
            if (isDropping && !jump) {
                isDropping = false;
                isSquashing = true;
                if (isReallyOnScreen()) {
                    gameObj->obf_7C->playSound("dillworm_land.wav", false, getProximityVolume(), getProximityPan());
                }
            }
        } else if (isHanging && !isNoticing) {
            if (player.y > y) {
                if (std::abs(player.x - x) < 32) {
                    isNoticing = true;
                    if (isReallyOnScreen()) {
                        gameObj->obf_7C->playSound("dillworm_notice.wav", false, getProximityVolume(),
                                                   getProximityPan());
                    }
                }
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
        updateSprite();
        if (dizzyFXID == -1 && isStunned && cycleFrame == cycleFrames("stunCycleFrames").loopFrame) {
            dizzyFXID = fx.addEffect(x, y - 30, "DizzyEffect", facingDir == 1 ? "faceRight" : "faceLeft");
        } else if (!isStunned && dizzyFXID != -1) {
            fx.removeEffect(dizzyFXID);
            dizzyFXID = -1;
        }
    }
}

// AS: Enemy17.findStartupSurface
void Enemy17::findStartupSurface()
{
    const managers::MapManager &map = *gameObj->obf_4i;
    const bool ceiling = map.getTileProperty(xtile, ytile - 1, "collision") != 0;
    const bool cloud = map.getTileProperty(xtile, ytile, "thrublock") != 0;
    jump = false;
    if (ceiling || cloud) {
        isHanging = true;
    } else {
        isHanging = false;
        fallChar();
    }
}

// AS: Enemy17.killMe
void Enemy17::killMe(double dir)
{
    Enemy::killMe(dir);
    if (gameObj->playerObj && gameObj->playerObj->playerData() &&
        gameObj->playerObj->playerData()->customerName == "Prudence") {
        gameObj->obf_94->recordTag("prupickle");
    }
}

} // namespace papaGame::models::enemies
