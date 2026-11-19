#include "game/papaGame/models/enemies/Enemy34.h"

#include <algorithm>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy34 constructor
void Enemy34::init(double xtile_, double ytile_, double id_, double whichScreen_, double whichTileset_,
                   double screenObjID_, const std::vector<as3::Value> &extraParams)
{
    Enemy::init(xtile_, ytile_, id_, whichScreen_, whichTileset_, screenObjID_, extraParams);
    startingXtile = xtile;
    if (targetXtile != -1) {
        dirx = startingXtile > targetXtile ? -1 : 1;
    }
}

// AS: Enemy34.defineVars
void Enemy34::defineVars()
{
    type = 34;
    sheetname = "enemysheet_burgerzilla";
    sheetIsBitmap = false;
    height = 15;
    width = 15;
    playerheight = 15;
    spritewidth = 74;
    spriteheight = 106;
    spriteCenterX = 37;
    spriteCenterY = 53;
    spriteTargetX = 37;
    spriteTargetY = 88;
    sheetWidth = 31;
    sheetHeight = 2;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 4;
    walkspeed = 4;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -20;
    flipBlitOffset = 106;
    collRect = {-25, -35, 50, 50};
    gravity = 2;
    normalgravity = 2;
    currentHealth = 4;
    maxHealth = 4;
    facingWalking = true;
    points = 10;
    canBeStunned = true;
    stunDuration = 90;
    rememberKilled = false;
    enemyCategory = "burgerzilla";
}

// AS: Enemy34.setupParams — [facing, rememberKilled, target tile, face the player, spawner enemy]
void Enemy34::setupParams(const std::vector<as3::Value> &params)
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
    if (params.size() > 3) {
        shouldFacePlayer = params[3].asNumber() == 1;
    }
    if (params.size() > 4) {
        spawnerObjectID = params[4].asNumber();
        jump = true;
        jumpspeed = -28;
    }
}

// AS: Enemy34.removeEnemy
void Enemy34::removeEnemy()
{
    if (spawnerObjectID != -1) {
        // SKELETON: the spawner enemy's unregisterSpawn(type, id) (no ported enemy spawns Burgerzillas yet).
    }
    Enemy::removeEnemy();
}

// AS: Enemy34.killMe
void Enemy34::killMe(double dir)
{
    if (!isDead) {
        gameObj->obf_51->killBurgerzilla();
    }
    // SKELETON: ChallengeManager.recordTag("martyzilla") when the player is Marty.
    Enemy::killMe(dir);
}

// AS: Enemy34.stunEnemy — a stomp is a hit.
void Enemy34::stunEnemy()
{
    if (!isStunned) {
        getHit(1, 1, false, "stomp");
        isStunned = true;
    }
}

// AS: Enemy34.getHit
bool Enemy34::getHit(double damage, double dir, bool unused, const std::string &weapon)
{
    const bool killed = Enemy::getHit(damage, dir, unused, weapon);
    if (isHit && !isDead) {
        isStunned = true;
        isSquashing = false;
        isReadyJump = false;
        jumpspeed = -12;
        jump = true;
        dirx = dir;
    }
    if (isHit && canDropParts) {
        gameObj->obf_V->addEffect(x, y, "ZillaPartsEffect",
                                  "piece" + as3::String(std::max(1.0, std::min(4.0, currentHealth + 1))));
        if (isDead) {
            canDropParts = false;
        }
    }
    return killed;
}

// AS: Enemy34.reactToHurtingPlayer
void Enemy34::reactToHurtingPlayer()
{
    isReactingToHurt = true;
    jumpspeed = -10;
    jump = true;
}

// AS: Enemy34.grabAnimationCycle — one set of cycles per number of ingredients left.
const AnimationCycle &Enemy34::grabAnimationCycle()
{
    if (isDead) {
        return cycleFrames("dieCycleFrames");
    }
    if (numIngredients > 0) {
        const std::string n = as3::String(numIngredients);
        if (isHit) {
            return cycleFrames((jump ? "jumphit" : "hit") + n + "CycleFrames");
        }
        if (isReadyJump) {
            return cycleFrames("readyjump" + n + "CycleFrames");
        }
        if (isSquashing) {
            return cycleFrames("squash" + n + "CycleFrames");
        }
        if (jump) {
            return cycleFrames("jump" + n + "CycleFrames");
        }
        return cycleFrames("stand" + n + "CycleFrames");
    }
    return cycleFrames("stand1CycleFrames");
}

// AS: Enemy34.endAnimationCycle
void Enemy34::endAnimationCycle()
{
    if (cycleName.starts_with("squash")) {
        isSquashing = false;
    } else if (cycleName.starts_with("readyjump")) {
        isReadyJump = false;
        jumpspeed = jumpstart;
        jump = true;
        if (isReallyOnScreen()) {
            gameObj->obf_7C->playSound("burgerzilla_jump.wav", false, getProximityVolume(), getProximityPan());
        }
    } else if (cycleName.starts_with("hit")) {
        isStunned = false;
        isHit = false;
    }
}

// AS: Enemy34.updateObject — hops toward its target tile (or the player), crouching before each jump.
void Enemy34::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    const auto &p = *gameObj->playerObj;
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
        return;
    }
    numIngredients = std::max(1.0, currentHealth);
    if (!checkOnScreen() && spawnerObjectID == -1) {
        return;
    }
    const bool wasJumping = jump;
    if (!isSquashing && !isReadyJump && jump) {
        moveChar(dirx, 0, 0, true);
    }
    fallChar();
    if (jump) {
        jumpChar();
    }
    if (shouldFacePlayer && !jump && !isHit) {
        dirx = x > p.x ? -1 : 1;
    } else if (!shouldFacePlayer && targetXtile != -1 && !isHit && !jump) {
        if (dirx == 1 && x > std::max(startingXtile, targetXtile) * d.tileWidth + d.tileWidth / 2) {
            dirx = -1;
        } else if (dirx == -1 && x < std::min(startingXtile, targetXtile) * d.tileWidth + d.tileWidth / 2) {
            dirx = 1;
        }
    }
    if (!isHit) {
        facingDir = dirx;
    }
    walkingDir = dirx;
    if (!jump && !isStunned && !isUnstunning && !isHit) {
        if (wasJumping) {
            isSquashing = true;
            if (isReallyOnScreen()) {
                gameObj->obf_7C->playSound("burgerzilla_land.wav", false, getProximityVolume(), getProximityPan());
            }
        }
        if (!isSquashing && !isReadyJump) {
            ++delayTimer;
            if (delayTimer >= delayBeforeJump) {
                delayTimer = 0;
                speed = walkspeed;
                isReadyJump = true;
            }
        }
    }
    if (isReactingToHurt && !jump) {
        isReactingToHurt = false;
    }
    updateSprite();
}

// AS: Enemy34.checkOnScreen — a wider margin than the other enemies.
bool Enemy34::checkOnScreen()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    if (xtile >= d.currentXtileLeft - 2 && xtile <= d.currentXtileRight + 2) {
        return ytile <= d.currentYtileDown + 2 && ytile >= d.currentYtileUp - 2;
    }
    return false;
}

// AS: Enemy34.jumpChar — only dies below the level while falling.
void Enemy34::jumpChar(bool collide)
{
    Enemy::jumpChar(collide); // AS: super.jumpChar, which already kills below the level
    if (ytile > gameObj->obf_71->levelTileHeight) {
        if (!isDead && jumpspeed > 0) {
            killMe(-1);
        }
    }
}

} // namespace papaGame::models::enemies
