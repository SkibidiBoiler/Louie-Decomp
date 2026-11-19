#include "game/papaGame/models/objects/GameObject7.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::objects {

// AS: GameObject7.GameObject7 — it heads for its target column first.
void GameObject7::init(double xtile_, double ytile_, double id_, double whichScreen_, double screenObjID_,
                       const std::vector<as3::Value> &extraParams)
{
    GameObject::init(xtile_, ytile_, id_, whichScreen_, screenObjID_, extraParams);
    startingXtile = xtile;
    if (startingXtile > targetXtile) {
        dirx = -1;
    } else {
        dirx = 1;
    }
}

// AS: GameObject7.defineVars
void GameObject7::defineVars()
{
    sheetname = "object_blimp_horizontal_small";
    objectname = "Horizonal Blimp Small";
    sheetIsBitmap = false;
    type = 7;
    spritewidth = 64;
    spriteheight = 73;
    spriteCenterX = 32;
    spriteCenterY = 36;
    spriteTargetX = 32;
    spriteTargetY = 32;
    sheetWidth = 14;
    sheetHeight = 1;
    collRect = {-20, -16, 40, 50};
    width = 15;
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
    walkspeed = 2;
}

// AS: GameObject7.setupParams — [wait for the player, target column]
void GameObject7::setupParams(const std::vector<as3::Value> &params)
{
    if (params.empty()) {
        return;
    }
    waitForPlayer = params[0].asNumber() == 1;
    targetXtile = params.size() > 1 ? params[1].asNumber() : as3::Number("");
}

// AS: GameObject7.endAnimationCycle
void GameObject7::endAnimationCycle()
{
    if (cycleName == "startstand") {
        playerIsRiding = true;
    } else if (cycleName == "stopstand") {
        playerIsRiding = false;
    }
}

// AS: GameObject7.grabAnimationCycle — it sinks when stood on and rises when left.
const AnimationCycle &GameObject7::grabAnimationCycle()
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

// AS: GameObject7.checkOnScreen — four columns either side of the screen.
bool GameObject7::checkOnScreen()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 4 && xtile < d.currentXtileRight + 4;
}

// AS: GameObject7.stepOnObject — it notes when the player boards.
void GameObject7::stepOnObject(Character &c)
{
    GameObject::stepOnObject(c);
    if (c.whichMovingTile == this && c.isPlayer()) {
        SDL_Log("Got a player");
        hasPlayer = true;
    }
}

// AS: GameObject7.stopStandingOnObject
void GameObject7::stopStandingOnObject(Character &c)
{
    GameObject::stopStandingOnObject(c);
    if (c.whichMovingTile != this && c.isPlayer()) {
        SDL_Log("Lost a player.");
        hasPlayer = false;
    }
}

// AS: GameObject7.updateObject — once moving it speeds up from a stop, turns at the ends of its columns (one that
// waits for the player stops back at its start if the player has left) and carries its riders; standing, it bobs.
void GameObject7::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    lastx = x;
    lasty = y;
    lastOnSlope = onSlope;
    if (isHit) {
        return;
    }
    if ((hasPlayer || waitForPlayer == false) && !isMoving) {
        isMoving = true;
    }
    if (isMoving) {
        if (speed < walkspeed) {
            if (std::fmod(gameObj->obf_8o->gameplayTimer, 2) == 0) {
                ++speed;
            }
        }
        const double startX = x;
        const double moveDir = dirx;
        moveChar(dirx, 0, 0, true);
        const double right = std::max(startingXtile, targetXtile) * d.tileWidth + d.tileWidth / 2;
        const double left = std::min(startingXtile, targetXtile) * d.tileWidth + d.tileWidth / 2;
        if (dirx == 1 && x > right) {
            x = right;
            xtile = std::floor(x / d.tileWidth);
            dirx = -1;
            speed = 0;
            if (xtile == startingXtile && !hasPlayer && waitForPlayer) {
                SDL_Log("Stop this waiter 1");
                isMoving = false;
            }
        } else if (dirx == -1 && x < left) {
            x = left;
            xtile = std::floor(x / d.tileWidth);
            dirx = 1;
            speed = 0;
            if (xtile == startingXtile && !hasPlayer && waitForPlayer) {
                SDL_Log("Stop this waiter -1");
                isMoving = false;
            }
        }
        // Its riders move as far as it did (4 less when they walk the same way).
        const double moved = std::abs(x - startX);
        for (int i = static_cast<int>(standingChars.size()) - 1; i >= 0; --i) {
            Character *c = standingChars[static_cast<size_t>(i)];
            double carry = moved;
            if (c->isWalking && c->walkingDir == dirx) {
                carry -= 4;
            }
            if (carry > 0) {
                c->moveChar(moveDir, 0, 0, false, false, true, carry);
            }
            standingCharsLastX[static_cast<size_t>(i)] = c->x;
        }
    }
    if (!isMoving) {
        ++waveTimer;
        y = initialY + std::sin(waveTimer / wavePeriod) * waveAmplitude;
        ytile = std::floor(y / d.tileWidth);
    }
    updateSprite();
    standOffset = spriteFrameOffsets[static_cast<size_t>(animationFrame)];
    for (int i = static_cast<int>(standingChars.size()) - 1; i >= 0; --i) {
        Character *c = standingChars[static_cast<size_t>(i)];
        c->y = y - spriteTargetY - c->height + standOffset;
        c->ytile = std::floor(c->y / d.tileWidth);
    }
}

} // namespace papaGame::models::objects
