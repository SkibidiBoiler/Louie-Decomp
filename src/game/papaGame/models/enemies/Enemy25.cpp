#include "game/papaGame/models/enemies/Enemy25.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy25.defineVars
void Enemy25::defineVars()
{
    type = 25;
    sheetname = "enemysheet_swisszack";
    sheetIsBitmap = false;
    height = 15;
    width = 15;
    playerheight = 15;
    spritewidth = 64;
    spriteheight = 216;
    spriteCenterX = 32;
    spriteCenterY = 108;
    spriteTargetX = 32;
    spriteTargetY = 10;
    sheetWidth = 17;
    sheetHeight = 1;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 2;
    walkspeed = 2;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -12;
    flipBlitOffset = 0;
    collRect = {-15, -20, 30, 40};
    gravity = 2;
    normalgravity = 2;
    currentHealth = 1;
    maxHealth = 1;
    facingWalking = true;
    points = 10;
    canBeStunned = false;
    stunDuration = 90;
    rememberKilled = false;
    enemyCategory = "swisszack";
}

// AS: Enemy25.setupParams — [starts up (1) or down, frames up, frames down (0: random), starting timer].
void Enemy25::setupParams(const std::vector<as3::Value> &params)
{
    if (params.size() > 0) {
        isDown = params[0].asNumber() != 1;
    }
    if (params.size() > 1) {
        upTimeMax = params[1].asNumber();
    }
    if (params.size() > 2) {
        downTimeMax = params[2].asNumber();
    }
    if (params.size() > 3) {
        stateTimer = params[3].asNumber();
    }
}

// AS: Enemy25.grabAnimationCycle
const AnimationCycle &Enemy25::grabAnimationCycle()
{
    if (isDropping) {
        return cycleFrames("dropCycleFrames");
    }
    if (isRising) {
        return cycleFrames("raiseCycleFrames");
    }
    if (isDown) {
        return cycleFrames("downCycleFrames");
    }
    return cycleFrames("upCycleFrames");
}

// AS: Enemy25.endAnimationCycle — a thump when it lands.
void Enemy25::endAnimationCycle()
{
    if (cycleName == "drop") {
        isDropping = false;
        isDown = true;
        if (isReallyOnScreen()) {
            gameObj->obf_7C->playSound("swisszack_thump.wav", false, getProximityVolume(), getProximityPan());
        }
    } else if (cycleName == "raise") {
        isRising = false;
        isDown = false;
    }
}

// AS: Enemy25.updateObject — it runs everywhere (the original tests `Boolean(true) || ...`). Its hit box follows
// the crusher down the animation. The original assigns upRect to collRect by reference, so setting collRect.y also
// changes upRect; y is set again every frame, so that changes nothing.
void Enemy25::updateObject()
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
    collRect = upRect;
    if (isDown && !isDropping && !isRising && !isStunned) {
        ++stateTimer;
        if (downTimeMax == 0) {
            if (as3::random() > 0.98) {
                isRising = true;
                stateTimer = 0;
            }
        } else if (stateTimer >= downTimeMax) {
            isRising = true;
            stateTimer = 0;
            isDown = false; // QUIRK: only the timed rise clears isDown here (the random one waits for "raise" to end).
        }
    } else if (!isDown && !isDropping && !isRising && !isStunned) {
        ++stateTimer;
        if (upTimeMax == 0) {
            if (as3::random() > 0.98) {
                isDropping = true;
                stateTimer = 0;
            }
        } else if (stateTimer >= upTimeMax) {
            isDropping = true;
            stateTimer = 0;
        }
    }
    updateSprite();
    collRect.y = collYoffsets[static_cast<size_t>(animationFrame)];
    if (checkOnScreen() && !didTrack) {
        saveEnemyKill();
        didTrack = true;
    }
}

// AS: Enemy25.checkOnScreen — it reaches 7 tiles below its own.
bool Enemy25::checkOnScreen()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 2 && xtile < d.currentXtileRight + 2 && ytile < d.currentYtileDown + 2 &&
           ytile + 7 > d.currentYtileUp - 2;
}

} // namespace papaGame::models::enemies
