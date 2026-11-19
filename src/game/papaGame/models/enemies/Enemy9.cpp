#include "game/papaGame/models/enemies/Enemy9.h"

#include <algorithm>
#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy9.Enemy9 — it starts at a random point of its walk.
void Enemy9::init(double xtile_, double ytile_, double id_, double whichScreen_, double whichTileset_,
                  double screenObjID_, const std::vector<as3::Value> &extraParams)
{
    Enemy::init(xtile_, ytile_, id_, whichScreen_, whichTileset_, screenObjID_, extraParams);
    const AnimationCycle &walk = cycleFrames("walkCycleFrames");
    cycleFrame = std::floor(as3::random() * static_cast<double>(walk.frames.size()));
    animationFrame = walk.frameAt(cycleFrame);
    cycleName = "walk";
}

// AS: Enemy9.defineVars
void Enemy9::defineVars()
{
    type = 9;
    sheetname = "enemysheet_partysubback";
    sheetIsBitmap = false;
    saveKillAsID = 11;
    height = 15;
    width = 15;
    playerheight = 15;
    spritewidth = 46;
    spriteheight = 60;
    spriteCenterX = 23;
    spriteCenterY = 30;
    spriteTargetX = 23;
    spriteTargetY = 44;
    sheetWidth = 51;
    sheetHeight = 2;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 2;
    walkspeed = 2;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -12;
    flipBlitOffset = 60;
    collRect = {-15, -20, 30, 40};
    gravity = 2;
    normalgravity = 2;
    currentHealth = 1;
    maxHealth = 1;
    facingWalking = true;
    points = 10;
    canBeStunned = true;
    stunDuration = 90;
    enemyCategory = "partysub";
}

// AS: Enemy9.setupParams — [facing, rememberKilled, group, spawner object, (spawned)]
void Enemy9::setupParams(const std::vector<as3::Value> &params)
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
        groupID = params[2].asNumber();
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

// AS: Enemy9.receiveGroupMessage — it keeps track of which parts of its sub are stunned.
void Enemy9::receiveGroupMessage(const std::string &message, double value)
{
    auto it = std::find(stunnedIDs.begin(), stunnedIDs.end(), value);
    if (message == "stun") {
        if (it == stunnedIDs.end()) {
            stunnedIDs.push_back(value);
        }
    } else if (message == "unstun") {
        if (it != stunnedIDs.end()) {
            stunnedIDs.erase(it);
        }
    }
}

// AS: Enemy9.stunEnemy — and it tells the rest of its sub.
void Enemy9::stunEnemy()
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
        gameObj->obf_94->recordTag("enemyStun");
        didTagStun = true;
    }
    gameObj->obf_E->sendMessageToEnemyGroup(id, groupID, "stun");
}

// AS: Enemy9.grabAnimationCycle
const AnimationCycle &Enemy9::grabAnimationCycle()
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

// AS: Enemy9.endAnimationCycle
void Enemy9::endAnimationCycle()
{
    if (cycleName == "turn") {
        isTurning = false;
    } else if (cycleName == "unstun") {
        jumpspeed = -10;
        jump = true;
        isJumpUnstunning = true;
    }
}

// AS: Enemy9.removeEnemy
void Enemy9::removeEnemy()
{
    if (spawnerObjectID != -1) {
        if (GameObject *spawner = gameObj->obf_7u->getObject(spawnerObjectID, whichScreen)) {
            spawner->unregisterSpawn();
        }
    }
    Enemy::removeEnemy();
}

// AS: Enemy9.updateObject — it moves while any part of its sub is on screen, and waits while one is stunned.
void Enemy9::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    managers::EnemyManager &manager = *gameObj->obf_E;
    managers::EffectManager &fx = *gameObj->obf_V;
    speed = adjustSpeedForSlopes(walkspeed);
    if (!gotGroupIDs) {
        groupEnemyIDs = manager.getEnemyIDsFromGroup(id, groupID);
        gotGroupIDs = true;
    }
    if (isDead) {
        jumpChar(false);
        moveChar(dirx, 0, 0, false, false, false);
        if (dizzyFXID != -1) {
            fx.removeEffect(dizzyFXID);
            dizzyFXID = -1;
        }
        if (isStunned) {
            manager.sendMessageToEnemyGroup(id, groupID, "unstun");
            isStunned = false;
        }
        updateSprite();
        if (ytile > d.currentYtileDown + 1) {
            removeEnemy();
        } else if (isLaunching && (ytile <= d.currentYtileUp - 1 || ytile < 0)) {
            removeEnemy();
        }
    } else if (manager.checkGroupOnScreen(groupEnemyIDs)) {
        const double lastDirX = dirx;
        if (!isStunned && !isUnstunning && !isReactingToHurt && stunnedIDs.empty()) {
            moveChar(dirx, 0, 0, true, true);
            isWalking = true;
        } else if (!stunnedIDs.empty()) {
            isWalking = false;
        }
        fallChar();
        if (jump) {
            jumpChar();
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
            manager.sendMessageToEnemyGroup(id, groupID, "unstun");
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

// AS: Enemy9.adjustFrameSync — walking (not out of a turn) and standing start at a random frame, so the parts of a
// sub do not move in step.
void Enemy9::adjustFrameSync(const std::string &newName, const std::string &oldName, double firstFrame,
                             const AnimationCycle &cycle)
{
    if (newName != oldName) {
        if ((newName == "walk" && oldName != "turn") || newName == "stand") {
            cycleFrame = std::floor(as3::random() * static_cast<double>(cycle.frames.size()));
            animationFrame = cycle.frameAt(cycleFrame);
            cycleName = newName;
        } else {
            animationFrame = firstFrame;
            cycleFrame = 0;
            cycleName = newName;
        }
    }
}

} // namespace papaGame::models::enemies
