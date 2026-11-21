#include "game/papaGame/models/enemies/Enemy22.h"

#include <algorithm>
#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy22.Enemy22 — a walker heads for its target column first.
void Enemy22::init(double xtile_, double ytile_, double id_, double whichScreen_, double whichTileset_,
                   double screenObjID_, const std::vector<as3::Value> &extraParams)
{
    Enemy::init(xtile_, ytile_, id_, whichScreen_, whichTileset_, screenObjID_, extraParams);
    startingXtile = xtile;
    if (!isStatic) {
        dirx = startingXtile <= targetXtile ? 1 : -1;
    }
}

// AS: Enemy22.defineVars
void Enemy22::defineVars()
{
    type = 22;
    sheetname = "enemysheet_bacobar";
    sheetIsBitmap = false;
    height = 15;
    width = 15;
    playerheight = 15;
    spritewidth = 50;
    spriteheight = 140;
    spriteCenterX = 25;
    spriteCenterY = 70;
    spriteTargetX = 25;
    spriteTargetY = 124;
    sheetWidth = 32;
    sheetHeight = 4;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 2;
    walkspeed = 2;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -12;
    flipBlitOffset = 280;
    collRect = {-15, -100, 30, 116};
    stompRect = {-15, -110, 30, 24};
    hasStompRect = true;
    gravity = 2;
    normalgravity = 2;
    currentHealth = 1;
    maxHealth = 1;
    facingWalking = true;
    points = 10;
    canBeStunned = true;
    stunDuration = 90;
    bouncingDoesStun = false;
    shoveAmount = 10;
    enemyCategory = "bacon";
}

// AS: Enemy22.setupParams — [facing, rememberKilled, static, target column, starts crunched].
void Enemy22::setupParams(const std::vector<as3::Value> &params)
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
        isStatic = params[2].asNumber() == 1;
        isWalking = !isStatic;
    }
    if (params.size() > 3) {
        targetXtile = params[3].asNumber();
    }
    if (params.size() > 4) {
        if (params[4].asNumber() == 1) {
            isCrunched = true;
            walkspeed = crunchSpeed;
            collRect = {-15, -50, 30, 66};
            hasStompRect = false;
        } else {
            isCrunched = false;
        }
    }
}

// AS: Enemy22.getHit — only a crunched one can be hurt; otherwise it wobbles.
bool Enemy22::getHit(double damage, double dir, bool unused, const std::string &weapon)
{
    if (isCrunched) {
        return Enemy::getHit(damage, dir, unused, weapon);
    }
    if (!isCrunching) {
        if (isReallyOnScreen() && !isStunned) {
            gameObj->obf_7C->playSound("bacon_wiggle.wav", false, getProximityVolume(), getProximityPan());
        }
        isStunned = true;
    }
    return false;
}

// AS: Enemy22.saveEnemyState — a remembered one comes back crunched, walking (target column 10).
void Enemy22::saveEnemyState()
{
    std::vector<as3::Value> row{as3::Value::num(type), as3::Value::num(xtile), as3::Value::num(ytile)};
    row.push_back(as3::Value::num(facingDir));
    row.push_back(as3::Value::num(1));
    row.push_back(as3::Value::num(0));
    row.push_back(as3::Value::num(10));
    row.push_back(as3::Value::num(1));
    gameObj->obf_AQ->currentScreenData->saveEnemyState(id - 1, row);
}

// AS: Enemy22.stunEnemy — the first stomp crunches it; on a crunched one a stomp is a hit.
void Enemy22::stunEnemy()
{
    if (!isCrunched && !isCrunching) {
        isCrunching = true;
        walkspeed = crunchSpeed;
        isStunned = true;
        collRect = {-15, -50, 30, 66};
        hasStompRect = false;
        isWalking = true;
        isStatic = false;
        gameObj->obf_7C->playSound("bacobar_crunch.wav", false, getProximityVolume(), getProximityPan());
        if (rememberKilled) {
            saveEnemyState();
        }
    } else if (isCrunched) {
        getHit(1, gameObj->playerObj->x > x ? -1 : 1, false);
    }
}

// AS: Enemy22.reactToHurtingPlayer
void Enemy22::reactToHurtingPlayer()
{
    if (isCrunched) {
        isReactingToHurt = true;
        jumpspeed = -10;
        jump = true;
    } else {
        isStunned = true;
    }
}

// AS: Enemy22.grabAnimationCycle
const AnimationCycle &Enemy22::grabAnimationCycle()
{
    if (isDead) {
        return cycleFrames("dieCycleFrames");
    }
    if (isCrunching) {
        return cycleFrames("crunchCycleFrames");
    }
    if (isStunned && !isCrunched) {
        return cycleFrames("wobbleCycleFrames");
    }
    if (isTurning) {
        return cycleFrames(isCrunched ? "turncrunchCycleFrames" : "turnCycleFrames");
    }
    if (isWalking) {
        return cycleFrames(isCrunched ? "walkcrunchCycleFrames" : "walkCycleFrames");
    }
    return cycleFrames("standCycleFrames");
}

// AS: Enemy22.endAnimationCycle
void Enemy22::endAnimationCycle()
{
    if (cycleName == "turn" || cycleName == "turncrunch") {
        isTurning = false;
    } else if (cycleName == "crunch") {
        isCrunching = false;
        isCrunched = true;
        isStunned = false;
    } else if (cycleName == "wobble") {
        isStunned = false;
        animationFrame = 62;
        cycleFrame = 0;
        cycleName = "stand";
    }
}

// AS: Enemy22.removeEnemy
void Enemy22::removeEnemy()
{
    if (spawnerObjectID != -1) {
        if (GameObject *spawner = gameObj->obf_7u->getObject(spawnerObjectID, whichScreen)) {
            spawner->unregisterSpawn();
        }
    }
    Enemy::removeEnemy();
}

// AS: Enemy22.updateObject — it speeds up again after each turn (one more every other frame) and turns at the ends
// of its run; an uncrunched one pushes a player inside it out 16 px past its side and hurts them.
void Enemy22::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    characters::CustomerChar &p = *gameObj->playerObj;
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
    // QUIRK: speed was just set to the full walkspeed above, so this never speeds it up.
    if (speed < walkspeed && std::fmod(gameObj->obf_8o->gameplayTimer, 2) == 0) {
        ++speed;
    }
    if (!isStunned && !isUnstunning && !isReactingToHurt && !isCrunching && !isStatic) {
        moveChar(dirx, 0, 0, true, true);
    }
    fallChar();
    if (jump) {
        jumpChar();
    }
    const double rightEndX = std::max(startingXtile, targetXtile) * d.tileWidth + d.tileWidth / 2;
    const double leftEndX = std::min(startingXtile, targetXtile) * d.tileWidth + d.tileWidth / 2;
    if (dirx == 1 && x > rightEndX) {
        x = rightEndX;
        xtile = std::floor(x / d.tileWidth);
        dirx = -1;
        speed = 0;
    } else if (dirx == -1 && x < leftEndX) {
        x = leftEndX;
        xtile = std::floor(x / d.tileWidth);
        dirx = 1;
        speed = 0;
    }
    facingDir = dirx;
    walkingDir = dirx;
    if (dirx != oldDir) {
        isTurning = true;
    }
    if (isReactingToHurt && !jump) {
        isReactingToHurt = false;
    }
    if (!isCrunched && !isCrunching && p.checkSpriteCollision(getCurrentCollRect())) {
        if (p.x < x) {
            p.x = x + collRect.x - 16;
            p.xtile = std::floor(p.x / d.tileWidth);
            if (p.canGetHit()) {
                p.hurtPlayer(-1, 1);
            }
        } else {
            p.x = x + collRect.x + collRect.width + 16;
            p.xtile = std::floor(p.x / d.tileWidth);
            if (p.canGetHit()) {
                p.hurtPlayer(1, 1);
            }
        }
    }
    updateSprite();
}

} // namespace papaGame::models::enemies
