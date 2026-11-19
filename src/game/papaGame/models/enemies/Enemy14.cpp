#include "game/papaGame/models/enemies/Enemy14.h"

#include <algorithm>
#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy14.Enemy14 — it flies towards its target row first.
void Enemy14::init(double xtile_, double ytile_, double id_, double whichScreen_, double whichTileset_,
                   double screenObjID_, const std::vector<as3::Value> &extraParams)
{
    Enemy::init(xtile_, ytile_, id_, whichScreen_, whichTileset_, screenObjID_, extraParams);
    startingYtile = ytile;
    if (startingYtile > targetYtile) {
        diry = -1;
    } else {
        diry = 1;
    }
}

// AS: Enemy14.defineVars
void Enemy14::defineVars()
{
    type = 14;
    sheetname = "enemysheet_lettucelark";
    sheetIsBitmap = false;
    height = 15;
    width = 15;
    playerheight = 15;
    spritewidth = 104;
    spriteheight = 68;
    spriteCenterX = 52;
    spriteCenterY = 34;
    spriteTargetX = 52;
    spriteTargetY = 52;
    sheetWidth = 21;
    sheetHeight = 4;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 4;
    walkspeed = 4;
    dirx = 0;
    jumpspeed = 0;
    jumpstart = -16;
    flipBlitOffset = 136;
    collRect = {-15, -20, 30, 30};
    gravity = 2;
    normalgravity = 2;
    currentHealth = 1;
    maxHealth = 1;
    facingWalking = true;
    points = 10;
    canBeStunned = true;
    stunDuration = 90;
    isFlying = true;
    enemyCategory = "lark";
}

// AS: Enemy14.setupParams — [facing, rememberKilled, target row]
void Enemy14::setupParams(const std::vector<as3::Value> &params)
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
        targetYtile = params[2].asNumber();
    }
}

// AS: Enemy14.stunEnemy — a stomp in the air knocks it down; on the ground it is stunned.
void Enemy14::stunEnemy()
{
    if (isFlying) {
        getHit();
    } else {
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

// AS: Enemy14.getHit — the first hit only plucks it out of the air (feathers fly); the hits during that fall do
// nothing.
bool Enemy14::getHit(double damage, double dir, bool unused, const std::string &weapon)
{
    if (isFlying) {
        isFlying = false;
        wasFlying = true;
        jump = true;
        jumpspeed = 0;
        speed = walkspeed;
        managers::EffectManager &fx = *gameObj->obf_V;
        fx.addEffect(x, y, "FeatherEffect", "piece1");
        fx.addEffect(x - 4, y - 3, "FeatherEffect", "piece2");
        fx.addEffect(x + 4, y - 2, "FeatherEffect", "piece3");
        fx.addEffect(x + 6, y + 4, "FeatherEffect", "piece4");
        fx.addEffect(x - 2, y + 5, "FeatherEffect", "piece2");
        fx.addEffect(x + 5, y - 6, "FeatherEffect", "piece1");
        gameObj->obf_7C->playSound("hitenemy.wav");
        return false;
    }
    if (!wasFlying) {
        return Enemy::getHit(damage, dir, unused, weapon);
    }
    return false;
}

// AS: Enemy14.reactToHurtingPlayer
void Enemy14::reactToHurtingPlayer()
{
    if (!isFlying) {
        isReactingToHurt = true;
        jumpspeed = -10;
        jump = true;
    }
}

// AS: Enemy14.grabAnimationCycle
const AnimationCycle &Enemy14::grabAnimationCycle()
{
    if (isDead) {
        return cycleFrames("dieCycleFrames");
    }
    if (isFlying) {
        return cycleFrames("flyCycleFrames");
    }
    if (isStunned && !wasFlying) {
        return cycleFrames("stunCycleFrames");
    }
    if (isSquashing) {
        return cycleFrames("landCycleFrames");
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
    return cycleFrames("standCycleFrames");
}

// AS: Enemy14.endAnimationCycle — QUIRK: it checks for a "turn" cycle it does not have.
void Enemy14::endAnimationCycle()
{
    if (cycleName == "turn") {
        isTurning = false;
    } else if (cycleName == "land") {
        isSquashing = false;
    } else if (cycleName == "unstun") {
        jumpspeed = -10;
        jump = true;
        isJumpUnstunning = true;
    }
}

// AS: Enemy14.removeEnemy
void Enemy14::removeEnemy()
{
    if (spawnerObjectID != -1) {
        if (GameObject *spawner = gameObj->obf_7u->getObject(spawnerObjectID, whichScreen)) {
            spawner->unregisterSpawn();
        }
    }
    Enemy::removeEnemy();
}

// AS: Enemy14.updateObject — in the air it speeds up from a stop and turns at the ends of its rows, flapping (with a
// sound on frames 4 and 13); on the ground it hops every delayBeforeJump frames.
void Enemy14::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
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
        if (isFlying) {
            if (speed < walkspeed) {
                if (std::fmod(gameObj->obf_8o->gameplayTimer, 2) == 0) {
                    ++speed;
                }
            }
            moveChar(0, diry, 0, true);
            const double low = std::max(startingYtile, targetYtile) * d.tileWidth + d.tileWidth / 2;
            const double high = std::min(startingYtile, targetYtile) * d.tileWidth + d.tileWidth / 2;
            if (diry == 1 && y > low) {
                y = low;
                ytile = std::floor(y / d.tileWidth);
                diry = -1;
                speed = 0;
            } else if (diry == -1 && y < high) {
                y = high;
                ytile = std::floor(y / d.tileWidth);
                diry = 1;
                speed = 0;
            }
            if (animationFrame == 4) {
                if (isReallyOnScreen()) {
                    if (lastFlap == 1) {
                        gameObj->obf_7C->playSound("wingflap2.wav", false, getProximityVolume(), getProximityPan());
                        lastFlap = 2;
                    } else {
                        gameObj->obf_7C->playSound("wingflap1.wav", false, getProximityVolume(), getProximityPan());
                        lastFlap = 1;
                    }
                }
            } else if (animationFrame == 13) {
                if (isReallyOnScreen()) {
                    gameObj->obf_7C->playSound("wingupflap.wav", false, getProximityVolume(), getProximityPan());
                }
            }
        } else {
            const bool wasJumping = jump;
            if (jump && !isSquashing && !isStunned && !isUnstunning && !isReactingToHurt && !wasFlying) {
                moveChar(dirx, 0, 0, true, false);
            }
            fallChar();
            if (jump) {
                jumpChar();
            }
            facingDir = dirx;
            walkingDir = dirx;
            if (!jump && !isStunned) {
                if (wasJumping) {
                    isSquashing = true;
                }
                if (!isSquashing) {
                    ++delayTimer;
                    if (delayTimer >= delayBeforeJump) {
                        jump = true;
                        jumpspeed = jumpstart;
                        delayTimer = 0;
                        gameObj->obf_7C->playSound("jump_roma.wav", false, getProximityVolume(), getProximityPan());
                    }
                }
            }
            if (wasFlying && !jump) {
                wasFlying = false;
                isStunned = false;
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
        }
        updateSprite();
        if (dizzyFXID == -1 && isStunned && cycleFrame == cycleFrames("stunCycleFrames").loopFrame) {
            dizzyFXID = gameObj->obf_V->addEffect(x, y - 30, "DizzyEffect", facingDir == 1 ? "faceRight" : "faceLeft");
        } else if (!isStunned && dizzyFXID != -1) {
            gameObj->obf_V->removeEffect(dizzyFXID);
            dizzyFXID = -1;
        }
    }
    if (facingDir == 1) {
        spriteTargetX = 58;
    } else {
        spriteTargetX = 46;
    }
}

} // namespace papaGame::models::enemies
