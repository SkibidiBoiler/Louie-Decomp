#include "game/papaGame/models/enemies/Enemy24.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy24.Enemy24 — one of the three bite shapes at random, thrown up a little to a random side.
void Enemy24::init(double xtile_, double ytile_, double id_, double whichScreen_, double whichTileset_,
                   double screenObjID_, const std::vector<as3::Value> &extraParams)
{
    Enemy::init(xtile_, ytile_, id_, whichScreen_, whichTileset_, screenObjID_, extraParams);
    whichBite = std::ceil(as3::random() * 3);
    animationFrame = (whichBite - 1) * 2;
    isStunned = true;
    jump = true;
    jumpspeed = 0 - std::round(as3::random() * 4);
    dirx = as3::random() > 0.5 ? 1 : -1;
}

// AS: Enemy24.defineVars
void Enemy24::defineVars()
{
    type = 24;
    sheetname = "enemysheet_bacobite";
    sheetIsBitmap = false;
    height = 15;
    width = 15;
    playerheight = 15;
    spritewidth = 36;
    spriteheight = 36;
    spriteCenterX = 18;
    spriteCenterY = 18;
    spriteTargetX = 18;
    spriteTargetY = 20;
    sheetWidth = 6;
    sheetHeight = 1;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 3;
    walkspeed = 3;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -14;
    flipBlitOffset = 0;
    collRect = {-10, -16, 20, 20};
    gravity = 1;
    normalgravity = 1;
    currentHealth = 1;
    maxHealth = 1;
    facingWalking = true;
    points = 10;
    canBeStunned = true;
    rememberKilled = false;
    bounceY = true;
    bounceDecayY = 0.3;
    enemyCategory = "bacon";
}

// AS: Enemy24.setupParams — [facing], turned the other way.
void Enemy24::setupParams(const std::vector<as3::Value> &params)
{
    if (params.size() > 0) {
        facingDir = params[0].asNumber() * -1;
        walkingDir = facingDir;
        dirx = walkingDir;
    }
}

// AS: Enemy24.stunEnemy — a stomp is a hit.
void Enemy24::stunEnemy()
{
    getHit(1, gameObj->playerObj->x > x ? -1 : 1, false);
}

// AS: Enemy24.grabAnimationCycle — the stand / die cycle of its bite shape.
const AnimationCycle &Enemy24::grabAnimationCycle()
{
    const std::string n = std::to_string(static_cast<int>(whichBite));
    if (isDead) {
        return cycleFrames("die" + n + "CycleFrames");
    }
    return cycleFrames("stand" + n + "CycleFrames");
}

// AS: Enemy24.updateObject — it moves only in the air; on the ground it waits 15 frames and hops the other way.
void Enemy24::updateObject()
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
    fallChar();
    if (jump) {
        jumpChar();
    }
    if (jump) {
        moveChar(dirx, 0, 0, true);
    }
    facingDir = dirx;
    walkingDir = dirx;
    if (!jump) {
        ++delayTimer;
        if (delayTimer >= delayBeforeJump) {
            dirx *= -1;
            facingDir = dirx;
            walkingDir = dirx;
            jump = true;
            jumpspeed = jumpstart;
            delayTimer = 0;
            if (isReallyOnScreen()) {
                gameObj->obf_7C->playSound("jump_bacobite.wav", false, getProximityVolume(), getProximityPan());
            }
        }
        if (isStunned) {
            isStunned = false;
        }
    }
    updateSprite();
}

} // namespace papaGame::models::enemies
