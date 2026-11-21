#include "game/papaGame/models/enemies/Enemy27.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy27.defineVars
void Enemy27::defineVars()
{
    type = 27;
    sheetname = "enemysheet_pepperjack";
    sheetIsBitmap = false;
    height = 15;
    width = 15;
    playerheight = 15;
    spritewidth = 63;
    spriteheight = 84;
    spriteCenterX = 32;
    spriteCenterY = 42;
    spriteTargetX = 32;
    spriteTargetY = 64;
    sheetWidth = 28;
    sheetHeight = 4;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 6;
    walkspeed = 6;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -18;
    flipBlitOffset = 168;
    collRect = {-15, -40, 30, 50};
    gravity = 2;
    normalgravity = 2;
    currentHealth = 1;
    maxHealth = 1;
    facingWalking = true;
    points = 10;
    canBeStunned = true;
    stunDuration = 90;
    enemyCategory = "cheese";
}

// AS: Enemy27.setupParams — [facing, rememberKilled].
void Enemy27::setupParams(const std::vector<as3::Value> &params)
{
    if (params.size() > 0) {
        facingDir = params[0].asNumber();
        walkingDir = facingDir;
        dirx = walkingDir;
    }
    if (params.size() > 1) {
        rememberKilled = params[1].asNumber() == 1;
    }
}

// AS: Enemy27.stunEnemy — a stun also stops it dead.
void Enemy27::stunEnemy()
{
    if (!isStunned) {
        isStunned = true;
    } else {
        cycleFrame = 0;
        animationFrame = cycleFrames("stunCycleFrames").frameAt(0);
    }
    walkspeed = 0;
    isUnstunning = false;
    isJumpUnstunning = false;
    stunStartTime = 0;
    if (!didTagStun) {
        gameObj->obf_94->recordTag("enemyStun");
        didTagStun = true;
    }
}

// AS: Enemy27.reactToHurtingPlayer — it hops back away from the player.
void Enemy27::reactToHurtingPlayer()
{
    isReactingToHurt = true;
    jumpspeed = -10;
    jump = true;
    dirx = gameObj->playerObj->x < x ? 1 : -1;
}

// AS: Enemy27.grabAnimationCycle
const AnimationCycle &Enemy27::grabAnimationCycle()
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
    if (isSkidding) {
        return cycleFrames("skidCycleFrames");
    }
    if (isNoticing) {
        return cycleFrames("noticeCycleFrames");
    }
    if (isTurning) {
        return cycleFrames("turnCycleFrames");
    }
    if (isWalking) {
        return cycleFrames("walkCycleFrames");
    }
    return cycleFrames("standCycleFrames");
}

// AS: Enemy27.endAnimationCycle — the notice ends in a charge.
void Enemy27::endAnimationCycle()
{
    if (cycleName == "turn") {
        isTurning = false;
    } else if (cycleName == "unstun") {
        jumpspeed = -10;
        jump = true;
        isJumpUnstunning = true;
    } else if (cycleName == "notice") {
        isNoticing = false;
        isChasing = true;
    }
}

// AS: Enemy27.updateObject — it notices a player within 160 x 96 px, then charges (one more speed every skidDecay
// frames, up to maxSpeed); 64 px past the player it skids, losing speed, and turns round. A charge that stops
// (a wall) becomes a jump towards the player, or a turn. 900 px away it gives up.
void Enemy27::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    managers::EffectManager &fx = *gameObj->obf_V;
    const characters::CustomerChar &p = *gameObj->playerObj;
    speed = adjustSpeedForSlopes(walkspeed);
    if (isDead) {
        walkspeed = 2;
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
    } else if (checkOnScreen() || spawnerObjectID != -1 || isChasing || isNoticing) {
        if (!isStunned && !isUnstunning && !isReactingToHurt) {
            ++skidTimer;
            if (isChasing && !isSkidding) {
                moveChar(dirx, 0, 0, false, true);
                if (std::fmod(skidTimer, skidDecay) == 0 && walkspeed < maxSpeed) {
                    ++walkspeed;
                }
                if (dirx == 1 && x - p.x >= 64) {
                    isSkidding = true;
                } else if (dirx == -1 && p.x - x >= 64) {
                    isSkidding = true;
                }
                if (!jump && (animationFrame == 15 || animationFrame == 25)) {
                    fx.addEffect(x - 6 * facingDir, y + 13, "DustEffect");
                }
            } else if (isSkidding) {
                moveChar(dirx, 0, 0, false, true);
                if (std::fmod(skidTimer, skidDecay) == 0) {
                    --walkspeed;
                    if (!jump) {
                        fx.addEffect(x - 6 * facingDir, y + 13, "DustEffect");
                    }
                }
                if (walkspeed <= 0) {
                    walkspeed = 0;
                    isSkidding = false;
                    isTurning = true;
                    dirx *= -1;
                }
            } else if (!isChasing && !isNoticing) {
                const double dx = p.x - x;
                const double dy = p.y - y;
                if (std::abs(dx) < 160 && std::abs(dy) < 96) {
                    const double dir = x < p.x ? 1 : -1;
                    facingDir = dir;
                    walkingDir = dir;
                    dirx = dir;
                    isNoticing = true;
                    walkspeed = 2;
                }
            }
        }
        fallChar();
        if (jump) {
            jumpChar();
        }
        if (isChasing && (dirx == 0 || !isWalking)) {
            if (!jump) {
                if ((p.x < x && facingDir == -1) || (p.x > x && facingDir == 1)) {
                    jumpspeed = jumpstart;
                    jump = true;
                    dirx = facingDir;
                    isWalking = true;
                    walkspeed = 4;
                } else {
                    dirx = facingDir * -1;
                    walkspeed = 4;
                    isWalking = true;
                    isSkidding = false;
                    isTurning = true;
                }
            } else {
                isWalking = true;
                dirx = facingDir;
                walkspeed = 4;
            }
        }
        facingDir = dirx;
        walkingDir = dirx;
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
        if (!jump && isChasing && std::abs(p.x - x) > 900) {
            isChasing = false;
            isNoticing = false;
        }
        if (dizzyFXID == -1 && isStunned && cycleFrame == cycleFrames("stunCycleFrames").loopFrame) {
            dizzyFXID = fx.addEffect(x, y - 30, "DizzyEffect", facingDir == 1 ? "faceRight" : "faceLeft");
        } else if (!isStunned && dizzyFXID != -1) {
            fx.removeEffect(dizzyFXID);
            dizzyFXID = -1;
        }
    }
    spriteTargetX = facingDir == 1 ? 28 : 36;
}

// AS: Enemy27.checkIfAtEdge — at a ledge it does not turn back: it jumps when the player is that way and not
// below it (and caps its speed at 7).
void Enemy27::checkIfAtEdge(double, double dir)
{
    const managers::MapManager &map = *gameObj->obf_4i;
    const characters::CustomerChar &p = *gameObj->playerObj;
    getMyCorners(x, y + 1);
    if (isWalking && !jump) {
        if ((!HitDownLeft || dir == 1) && (!HitDownRight || dir == -1) && !checkIfHalfOnCloud(dir) &&
            !checkMovingTiles(1) && onSlope == 0) {
            if (map.getTileProperty(xtile + dir, ytile + 1, "slope") == 0 &&
                map.getTileProperty(xtile + dir, ytile, "slope") == 0) {
                if (((dir == 1 && p.x > x) || (dir == -1 && p.x < x)) && p.y <= y) {
                    jumpspeed = jumpstart;
                    jump = true;
                    if (walkspeed > 7) {
                        walkspeed = 7;
                    }
                }
            }
        }
    }
}

// AS: Enemy27.killMe — Ninjoy beating one earns a tag.
void Enemy27::killMe(double dir)
{
    Enemy::killMe(dir);
    if (gameObj->playerObj && gameObj->playerObj->playerData()->customerName == "Ninjoy") {
        gameObj->obf_94->recordTag("ninjoypepperjack");
    }
}

} // namespace papaGame::models::enemies
