#include "game/papaGame/models/enemies/Enemy18.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy18.defineVars
void Enemy18::defineVars()
{
    type = 18;
    sheetname = "enemysheet_dillweed";
    sheetIsBitmap = false;
    height = 15;
    width = 15;
    playerheight = 15;
    spritewidth = 108;
    spriteheight = 76;
    spriteCenterX = 54;
    spriteCenterY = 38;
    spriteTargetX = 54;
    spriteTargetY = 20;
    sheetWidth = 13;
    sheetHeight = 6;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 2;
    walkspeed = 2;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -12;
    flipBlitOffset = 0;
    collRect = {-15, -15, 30, 30};
    gravity = 2;
    normalgravity = 2;
    currentHealth = 1;
    maxHealth = 1;
    facingWalking = true;
    points = 10;
    canBeStunned = true;
    stunDuration = 90;
    bouncingDoesStun = false;
    enemyCategory = "dill";
}

// AS: Enemy18.setupParams — [rememberKilled].
void Enemy18::setupParams(const std::vector<as3::Value> &params)
{
    if (params.size() > 0) {
        rememberKilled = params[0].asNumber() == 1;
    }
}

// AS: Enemy18.stunEnemy — a stun stops the chomp (a second one restarts the stun).
void Enemy18::stunEnemy()
{
    if (!isStunned) {
        isStunned = true;
    } else {
        cycleFrame = 0;
        animationFrame = cycleFrames("stunCycleFrames").frameAt(0);
    }
    isChomping = false;
    isUnchomping = false;
    isUnstunning = false;
    isJumpUnstunning = false;
    stunStartTime = 0;
}

// AS: Enemy18.grabAnimationCycle
const AnimationCycle &Enemy18::grabAnimationCycle()
{
    if (isDead) {
        return cycleFrames("dieCycleFrames");
    }
    if (isStunned) {
        return cycleFrames("stunCycleFrames");
    }
    if (isChomping) {
        return cycleFrames("chomp" + chompDirection + "CycleFrames");
    }
    if (isUnchomping) {
        return cycleFrames("unchomp" + chompDirection + "CycleFrames");
    }
    return cycleFrames("standCycleFrames");
}

// AS: Enemy18.endAnimationCycle
void Enemy18::endAnimationCycle()
{
    if (cycleName == "chompLeft" || cycleName == "chompRight" || cycleName == "chompForward") {
        isChomping = false;
        isUnchomping = true;
    } else if (cycleName == "unchompLeft" || cycleName == "unchompRight" || cycleName == "unchompForward") {
        isUnchomping = false;
    } else if (cycleName == "stun") {
        isStunned = false;
    }
}

// AS: Enemy18.updateObject — chomps again as soon as the last one ends, turning Right → Forward → Left → Right.
void Enemy18::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
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
    if (isChomping && cycleFrame >= 6) {
        collRect = chompDirection == "Forward" ? chompRectForward
                   : chompDirection == "Left"  ? chompRectLeft
                                               : chompRectRight;
    } else {
        collRect = normalCollRect;
    }
    if (!isChomping && !isUnchomping && !isStunned) {
        if (chompDirection == "Right") {
            chompDirection = "Forward";
        } else if (chompDirection == "Forward") {
            chompDirection = "Left";
        } else if (chompDirection == "Left") {
            chompDirection = "Right";
        }
        isChomping = true;
    }
    if (isChomping && cycleFrame == 10 && isReallyOnScreen()) {
        gameObj->obf_7C->playSound(chompDirection == "Forward" ? "dillweed_chomp.wav" : "dillweed_chomp2.wav", false,
                                   getProximityVolume(), getProximityPan());
    }
    updateSprite();
}

// AS: Enemy18.killMe — Prudence beating one earns a tag.
void Enemy18::killMe(double dir)
{
    Enemy::killMe(dir);
    if (gameObj->playerObj && gameObj->playerObj->playerData()->customerName == "Prudence") {
        gameObj->obf_94->recordTag("prupickle");
    }
}

} // namespace papaGame::models::enemies
