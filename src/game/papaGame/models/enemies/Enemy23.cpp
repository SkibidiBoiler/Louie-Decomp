#include "game/papaGame/models/enemies/Enemy23.h"

#include <algorithm>
#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy23.Enemy23 — it heads for its target column first.
void Enemy23::init(double xtile_, double ytile_, double id_, double whichScreen_, double whichTileset_,
                   double screenObjID_, const std::vector<as3::Value> &extraParams)
{
    Enemy::init(xtile_, ytile_, id_, whichScreen_, whichTileset_, screenObjID_, extraParams);
    startingXtile = xtile;
    dirx = startingXtile > targetXtile ? -1 : 1;
}

// AS: Enemy23.defineVars
void Enemy23::defineVars()
{
    type = 23;
    sheetname = "enemysheet_bacoburn";
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
    sheetWidth = 37;
    sheetHeight = 2;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 2;
    walkspeed = 2;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -12;
    flipBlitOffset = 140;
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
    enemyCategory = "bacon";
}

// AS: Enemy23.setupParams — [facing, rememberKilled, target column].
void Enemy23::setupParams(const std::vector<as3::Value> &params)
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

// AS: Enemy23.getHit — only a ground pound, a slide or a stomp hurts it; anything else makes it wobble.
bool Enemy23::getHit(double damage, double dir, bool unused, const std::string &weapon)
{
    if (weapon == "groundpound" || weapon == "slide" || weapon == "stomp") {
        const bool hit = Enemy::getHit(damage, dir, unused, weapon);
        if (hit) {
            gameObj->obf_7C->playSound("bacoburn_crunch.wav", false, getProximityVolume(), getProximityPan());
            addBacoBites();
        }
        return hit;
    }
    if (isReallyOnScreen() && !isStunned) {
        gameObj->obf_7C->playSound("bacon_wiggle.wav", false, getProximityVolume(), getProximityPan());
    }
    isStunned = true;
    return false;
}

// AS: Enemy23.stunEnemy — a stomp is a hit.
void Enemy23::stunEnemy()
{
    getHit(1, gameObj->playerObj->x > x ? -1 : 1, false, "stomp");
}

// AS: Enemy23.reactToHurtingPlayer
void Enemy23::reactToHurtingPlayer()
{
    isStunned = true;
}

// AS: Enemy23.addBacoBites — three bites, one per tile up its height. (The original also traces "Add Baco Bites
// Here." to the debug log.)
void Enemy23::addBacoBites()
{
    managers::EnemyManager &em = *gameObj->obf_E;
    em.addEnemy(xtile, ytile, 24, 1, 1, em.counter + 1);
    em.addEnemy(xtile, ytile - 1, 24, 1, 1, em.counter + 1);
    em.addEnemy(xtile, ytile - 2, 24, 1, 1, em.counter + 1);
}

// AS: Enemy23.grabAnimationCycle
const AnimationCycle &Enemy23::grabAnimationCycle()
{
    if (isDead) {
        return cycleFrames("dieCycleFrames");
    }
    if (isStunned) {
        return cycleFrames("wobbleCycleFrames");
    }
    if (isTurning) {
        return cycleFrames("turnCycleFrames");
    }
    if (isWalking) {
        return cycleFrames("walkCycleFrames");
    }
    return cycleFrames("standCycleFrames");
}

// AS: Enemy23.endAnimationCycle
void Enemy23::endAnimationCycle()
{
    if (cycleName == "turn") {
        isTurning = false;
    } else if (cycleName == "wobble") {
        isStunned = false;
        animationFrame = 0;
        cycleFrame = 0;
        cycleName = "stand";
    }
}

// AS: Enemy23.removeEnemy
void Enemy23::removeEnemy()
{
    if (spawnerObjectID != -1) {
        if (GameObject *spawner = gameObj->obf_7u->getObject(spawnerObjectID, whichScreen)) {
            spawner->unregisterSpawn();
        }
    }
    Enemy::removeEnemy();
}

// AS: Enemy23.updateObject — once beaten it goes at once (the bites take its place); it turns at the ends of its
// run and pushes a player inside it out 16 px past its side, hurting them.
void Enemy23::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    characters::CustomerChar &p = *gameObj->playerObj;
    speed = adjustSpeedForSlopes(walkspeed);
    if (isDead) {
        jumpChar(false);
        moveChar(dirx, 0, 0, false, false, false);
        updateSprite();
        removeEnemy();
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
    if (!isStunned && !isUnstunning && !isReactingToHurt && startingXtile != targetXtile) {
        moveChar(dirx, 0, 0, true, true);
    }
    fallChar();
    if (jump) {
        jumpChar();
    }
    if (startingXtile != targetXtile) {
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
    }
    facingDir = dirx;
    walkingDir = dirx;
    if (dirx != oldDir) {
        isTurning = true;
    }
    if (isReactingToHurt && !jump) {
        isReactingToHurt = false;
    }
    if (p.checkSpriteCollision(getCurrentCollRect())) {
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
