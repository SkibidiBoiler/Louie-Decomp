#include "game/papaGame/models/objects/GameObject6.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::objects {

// AS: GameObject6.GameObject6 — it heads for its target column first.
void GameObject6::init(double xtile_, double ytile_, double id_, double whichScreen_, double screenObjID_,
                       const std::vector<as3::Value> &extraParams)
{
    GameObject::init(xtile_, ytile_, id_, whichScreen_, screenObjID_, extraParams);
    if (!spawnedByBoss) {
        startingXtile = xtile;
    }
    if (startingXtile > targetXtile) {
        SDL_Log("Starting X > Target X (%s > %s) -- go LEFT (Current X = %s)", as3::String(startingXtile).c_str(),
                as3::String(targetXtile).c_str(), as3::String(xtile).c_str());
        dirx = -1;
    } else {
        SDL_Log("Starting X <= Target X (%s <= %s) -- go RIGHT (Current X = %s)", as3::String(startingXtile).c_str(),
                as3::String(targetXtile).c_str(), as3::String(xtile).c_str());
        dirx = 1;
    }
}

// AS: GameObject6.defineVars
void GameObject6::defineVars()
{
    sheetname = "object_blimp_horizontal";
    objectname = "Horizonal Blimp";
    sheetIsBitmap = false;
    type = 6;
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
    walkspeed = 3;
}

// AS: GameObject6.setupParams — [wait for the player, target column, spawned by a boss, starting column (flies in)]
void GameObject6::setupParams(const std::vector<as3::Value> &params)
{
    if (params.empty()) {
        return;
    }
    waitForPlayer = params[0].asNumber() == 1;
    targetXtile = params.size() > 1 ? params[1].asNumber() : as3::Number("");
    if (params.size() > 2) {
        spawnedByBoss = params[2].asNumber() == 1;
    }
    if (params.size() > 3) {
        startingXtile = params[3].asNumber();
        isTweeningIn = true;
        SDL_Log("BOSS-SPAWN: Starting X tile = %s. Target X tile = %s", as3::String(startingXtile).c_str(),
                as3::String(targetXtile).c_str());
    }
}

// AS: GameObject6.endAnimationCycle
void GameObject6::endAnimationCycle()
{
    if (cycleName == "startstand") {
        playerIsRiding = true;
    } else if (cycleName == "stopstand") {
        playerIsRiding = false;
    }
}

// AS: GameObject6.grabAnimationCycle — it sinks when stood on and rises when left.
const AnimationCycle &GameObject6::grabAnimationCycle()
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

// AS: GameObject6.checkOnScreen — four columns either side of the screen.
bool GameObject6::checkOnScreen()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 4 && xtile < d.currentXtileRight + 4;
}

// AS: GameObject6.stepOnObject — it notes when the player boards.
void GameObject6::stepOnObject(Character &c)
{
    GameObject::stepOnObject(c);
    if (c.whichMovingTile == this && c.isPlayer()) {
        SDL_Log("Got a player");
        hasPlayer = true;
    }
}

// AS: GameObject6.stopStandingOnObject
void GameObject6::stopStandingOnObject(Character &c)
{
    GameObject::stopStandingOnObject(c);
    if (c.whichMovingTile != this && c.isPlayer()) {
        SDL_Log("Lost a player.");
        hasPlayer = false;
    }
}

// AS: GameObject6.updateObject — once moving it speeds up from a stop, turns at the ends of its columns (one that
// waits for the player stops back at its start if the player has left) and carries its riders; standing, it bobs.
void GameObject6::updateObject()
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
        if (isTweeningIn) {
            moveChar(dirx, 0, 0, false, false, false);
        } else {
            moveChar(dirx, 0, 0, true);
        }
        if (!isTweeningIn) {
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
        } else if (xtile == startingXtile) {
            isTweeningIn = false;
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
