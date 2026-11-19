#include "game/papaGame/models/enemies/Enemy15.h"

#include <algorithm>
#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy15.Enemy15 — it flies towards its target column first.
void Enemy15::init(double xtile_, double ytile_, double id_, double whichScreen_, double whichTileset_,
                   double screenObjID_, const std::vector<as3::Value> &extraParams)
{
    Enemy::init(xtile_, ytile_, id_, whichScreen_, whichTileset_, screenObjID_, extraParams);
    startingXtile = xtile;
    initialY = y;
    if (startingXtile > targetXtile) {
        dirx = -1;
    } else {
        dirx = 1;
    }
}

// AS: Enemy15.defineVars
void Enemy15::defineVars()
{
    type = 15;
    sheetname = "enemysheet_leafylark";
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
    sheetWidth = 22;
    sheetHeight = 4;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 4;
    walkspeed = 4;
    diry = 0;
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

// AS: Enemy15.setupParams — [facing, rememberKilled, target column]
void Enemy15::setupParams(const std::vector<as3::Value> &params)
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
        targetXtile = params[2].asNumber();
    }
}

// AS: Enemy15.stunEnemy — a stomp in the air knocks it down; on the ground it is stunned.
void Enemy15::stunEnemy()
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

// AS: Enemy15.getHit — the first hit only plucks it out of the air (feathers fly); the hits during that fall do
// nothing.
bool Enemy15::getHit(double damage, double dir, bool unused, const std::string &weapon)
{
    if (isFlying) {
        isFlying = false;
        wasFlying = true;
        jump = true;
        jumpspeed = 0;
        speed = walkspeed;
        isTurning = false;
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

// AS: Enemy15.reactToHurtingPlayer
void Enemy15::reactToHurtingPlayer()
{
    if (!isFlying) {
        isReactingToHurt = true;
        jumpspeed = -10;
        jump = true;
    }
}

// AS: Enemy15.grabAnimationCycle
const AnimationCycle &Enemy15::grabAnimationCycle()
{
    if (isDead) {
        return cycleFrames("dieCycleFrames");
    }
    if (isTurning) {
        return cycleFrames("turnCycleFrames");
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

// AS: Enemy15.endAnimationCycle
void Enemy15::endAnimationCycle()
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

// AS: Enemy15.removeEnemy
void Enemy15::removeEnemy()
{
    if (spawnerObjectID != -1) {
        if (GameObject *spawner = gameObj->obf_7u->getObject(spawnerObjectID, whichScreen)) {
            spawner->unregisterSpawn();
        }
    }
    Enemy::removeEnemy();
}

// AS: Enemy15.updateObject — in the air it speeds up from a stop, turns at the ends of its columns and bobs on a sine
// wave, flapping (with a sound on frames 4 and 13); on the ground it hops every delayBeforeJump frames.
void Enemy15::updateObject()
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
            const double lastDirX = dirx;
            if (speed < walkspeed) {
                if (std::fmod(gameObj->obf_8o->gameplayTimer, 2) == 0) {
                    ++speed;
                }
            }
            moveChar(dirx, 0, 0, true);
            const double right = std::max(startingXtile, targetXtile) * d.tileWidth + d.tileWidth / 2;
            const double left = std::min(startingXtile, targetXtile) * d.tileWidth + d.tileWidth / 2;
            if (dirx == 1 && x > right) {
                x = right;
                xtile = std::floor(x / d.tileWidth);
                dirx = -1;
                speed = 0;
            } else if (dirx == -1 && x < left) {
                x = left;
                xtile = std::floor(x / d.tileWidth);
                dirx = 1;
                speed = 0;
            }
            facingDir = dirx;
            walkingDir = dirx;
            if (dirx != lastDirX) {
                isTurning = true;
            }
            ++waveTimer;
            y = initialY + std::sin(waveTimer / wavePeriod) * waveAmplitude;
            ytile = std::floor(y / d.tileWidth);
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
