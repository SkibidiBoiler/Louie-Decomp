#include "game/papaGame/models/objects/GameObject4.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::objects {

// AS: GameObject4.defineVars
void GameObject4::defineVars()
{
    sheetname = "object_blimp_static";
    objectname = "Static Blimp";
    sheetIsBitmap = false;
    type = 4;
    spritewidth = 96;
    spriteheight = 110;
    spriteCenterX = 48;
    spriteCenterY = 55;
    spriteTargetX = 48;
    spriteTargetY = 32;
    sheetWidth = 14;
    sheetHeight = 1;
    collRect = {-32, -16, 64, 80};
    width = 20;
    height = 15;
    playerheight = 15;
    isActivated = true;
    currentHealth = 3;
    maxHealth = 3;
    foreground = false;
    isSteppable = true;
    standingChars.clear();
    standingCharsLastX.clear();
    isPushable = false;
    isGrabbable = false;
    jumpspeed = 0;
    jumpstart = -16;
    jumpgravity = 2;
    normalgravity = 2;
    throwSpeed = 8;
    bounceY = false;
    statAttack = 500;
    canBeShot = false;
    canBeMeleed = false;
    canBeBrokenByObjects = false;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 0;
    walkspeed = 0;
}

// AS: GameObject4.setupParams — [spawned by a boss, row to tween in to]. QUIRK: the one in column 18 comes in at once.
void GameObject4::setupParams(const std::vector<as3::Value> &params)
{
    if (params.size() > 0) {
        spawnedByBoss = params[0].asNumber() == 1;
    }
    if (params.size() > 1) {
        targetYtile = params[1].asNumber();
        isTweeningIn = true;
        tweenInDelayTimerMax = std::ceil(as3::random() * 14);
        if (xtile == 18) {
            tweenInDelayTimerMax = 0;
        }
    }
}

// AS: GameObject4.destroy — a boss's blimp drops its riders and goes in a puff of dust.
void GameObject4::destroy()
{
    if (spawnedByBoss) {
        for (int i = static_cast<int>(standingChars.size()) - 1; i >= 0; --i) {
            stopStandingOnObject(*standingChars[static_cast<size_t>(i)]);
        }
        if (gameObj->obf_V) {
            gameObj->obf_V->addEffect(x - 8, y + 32, "DustEffect", "anim", false, -1, -1);
            gameObj->obf_V->addEffect(x, y + 16, "DustEffect", "anim", false, 0, -1);
            gameObj->obf_V->addEffect(x + 8, y + 32, "DustEffect", "anim", false, 1, -1);
        }
    }
}

// AS: GameObject4.endAnimationCycle
void GameObject4::endAnimationCycle()
{
    if (cycleName == "startstand") {
        playerIsRiding = true;
    } else if (cycleName == "stopstand") {
        playerIsRiding = false;
    }
}

// AS: GameObject4.grabAnimationCycle — it sinks when stood on and rises when left.
const AnimationCycle &GameObject4::grabAnimationCycle()
{
    if (!standingChars.empty() && !playerIsRiding) {
        return startstandCycleFrames;
    }
    if (standingChars.empty() && playerIsRiding) {
        return stopstandCycleFrames;
    }
    if (playerIsRiding) {
        return standCycleFrames;
    }
    return normalCycleFrames;
}

// AS: GameObject4.checkOnScreen — four columns either side of the screen.
bool GameObject4::checkOnScreen()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 4 && xtile < d.currentXtileRight + 4;
}

// AS: GameObject4.updateObject — it bobs on a sine wave (or tweens in to its row), and carries its riders.
void GameObject4::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    lastx = x;
    lasty = y;
    lastOnSlope = onSlope;
    if (!isHit) {
        if (!isTweeningIn) {
            ++waveTimer;
            y = std::round(initialY + std::sin(waveTimer / wavePeriod) * waveAmplitude);
            ytile = std::floor(y / d.tileWidth);
        } else {
            ++tweenInDelayTimer;
            if (tweenInDelayTimer >= tweenInDelayTimerMax) {
                const double targetY = targetYtile * d.tileWidth + d.tileWidth / 2;
                const double distance = targetY - y;
                y += distance / 6;
                ytile = std::floor(y / d.tileWidth);
                if (std::abs(distance) <= 2) {
                    y = targetY;
                    initialY = y;
                    ytile = std::floor(y / d.tileWidth);
                    isTweeningIn = false;
                }
            }
        }
        updateSprite();
        standOffset = spriteFrameOffsets[static_cast<size_t>(animationFrame)];
        for (int i = static_cast<int>(standingChars.size()) - 1; i >= 0; --i) {
            Character *c = standingChars[static_cast<size_t>(i)];
            c->y = y - spriteTargetY - c->height + standOffset;
            c->ytile = std::floor(c->y / d.tileWidth);
        }
    }
}

} // namespace papaGame::models::objects
