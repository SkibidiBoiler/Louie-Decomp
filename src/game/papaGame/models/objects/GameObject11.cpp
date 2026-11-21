#include "game/papaGame/models/objects/GameObject11.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"
#include "game/obf_AF/obf_1n.h"

namespace papaGame::models::objects {

// AS: GameObject11.GameObject11 — its placed position is the circle's centre.
void GameObject11::init(double xtile_, double ytile_, double id_, double whichScreen_, double screenObjID_,
                        const std::vector<as3::Value> &extraParams)
{
    GameObject::init(xtile_, ytile_, id_, whichScreen_, screenObjID_, extraParams);
    flightCircleX = x;
    flightCircleY = y;
}

// AS: GameObject11.defineVars
void GameObject11::defineVars()
{
    sheetname = "object_blimp_circular_small";
    objectname = "Circular Blimp Small";
    sheetIsBitmap = false;
    type = 11;
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
    speed = 2;
    walkspeed = 2;
}

// AS: GameObject11.setupParams — [wait for the player, radius in tiles, direction (1 or -1), start angle in degrees,
// tethered]. The timer starts at the frame matching the start angle.
void GameObject11::setupParams(const std::vector<as3::Value> &params)
{
    if (params.empty()) {
        return;
    }
    auto num = [&](size_t i) { return i < params.size() ? params[i].asNumber() : as3::NaN; };
    waitForPlayer = num(0) == 1;
    tileRadius = num(1);
    rotationDirection = num(2);
    blimpAngle = obf_AF::obf_1n::degreesToRads(num(3));
    rotationTimer = blimpAngle / (walkspeed / (tileRadius * gameObj->obf_71->tileWidth));
    isTethered = num(4) == 1;
}

// AS: GameObject11.endAnimationCycle
void GameObject11::endAnimationCycle()
{
    if (cycleName == "startstand") {
        playerIsRiding = true;
    } else if (cycleName == "stopstand") {
        playerIsRiding = false;
    }
}

// AS: GameObject11.grabAnimationCycle — it sinks when stood on and rises when left.
const AnimationCycle &GameObject11::grabAnimationCycle()
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

// AS: GameObject11.checkOnScreen — four columns either side of the screen.
bool GameObject11::checkOnScreen()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 4 && xtile < d.currentXtileRight + 4;
}

// AS: GameObject11.stepOnObject
void GameObject11::stepOnObject(Character &c)
{
    GameObject::stepOnObject(c);
    if (c.whichMovingTile == this && c.isPlayer()) {
        hasPlayer = true;
    }
}

// AS: GameObject11.stopStandingOnObject
void GameObject11::stopStandingOnObject(Character &c)
{
    GameObject::stopStandingOnObject(c);
    if (c.whichMovingTile != this && c.isPlayer()) {
        hasPlayer = false;
    }
}

// AS: GameObject11.updateObject — once moving (boarded, or never waiting) it keeps circling; its riders are carried
// sideways by its x move (less 4 px when they walk its way) and held on its deck.
void GameObject11::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    lastx = x;
    lasty = y;
    lastOnSlope = onSlope;
    if (isHit) {
        return;
    }
    if ((hasPlayer || !waitForPlayer) && !isMoving) {
        isMoving = true;
    }
    const double oldX = x;
    if (isMoving) {
        ++rotationTimer;
    }
    flyChar();
    const double newX = x;
    for (int i = static_cast<int>(standingChars.size()) - 1; i >= 0; --i) {
        Character *c = standingChars[static_cast<size_t>(i)];
        double carry = std::abs(newX - oldX);
        const double dir = newX < oldX ? -1 : 1;
        if (c->isWalking && c->walkingDir == dirx) {
            carry -= 4;
        }
        if (carry > 0) {
            c->moveChar(dir, 0, 0, false, false, true, carry);
        }
        standingCharsLastX[static_cast<size_t>(i)] = c->x;
    }
    updateSprite();
    standOffset = spriteFrameOffsets[static_cast<size_t>(animationFrame)];
    for (int i = static_cast<int>(standingChars.size()) - 1; i >= 0; --i) {
        Character *c = standingChars[static_cast<size_t>(i)];
        c->y = y - spriteTargetY - c->height + standOffset;
        c->ytile = std::floor(c->y / d.tileWidth);
    }
}

// AS: GameObject11.flyChar — the point on the circle for the timer (walkspeed px per frame), kept in the level.
void GameObject11::flyChar()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    const double tile = d.tileWidth;
    const double radius = tileRadius * tile;
    const double angleStep = walkspeed * rotationDirection / radius;
    x = std::round(flightCircleX + radius * std::cos(angleStep * rotationTimer));
    y = std::round(flightCircleY + radius * std::sin(angleStep * rotationTimer));
    const auto &tiles = gameObj->obf_AQ->currentScreenData->tileArray;
    const double maxX = static_cast<double>(tiles[0].size()) * tile;
    const double maxY = static_cast<double>(tiles.size()) * tile;
    if (x < 0) {
        x = 0;
    }
    if (x > maxX) {
        x = maxX;
    }
    if (y < 0) {
        y = 0;
    }
    if (y > maxY) {
        y = maxY;
    }
    xtile = std::floor(x / tile);
    ytile = std::floor(y / tile);
}

// AS: GameObject11.checkAdditionalBlitting — a tethered blimp draws, before itself, a rope (4 px 0x222222 with a
// 2 px 0x999999 core) from below it to below the circle's centre. Port: the blimp end follows its drawn
// (interpolated) position.
void GameObject11::checkAdditionalBlitting(bool foregroundPass, bool before)
{
    const display::GameDisplay &d = *gameObj->obf_71;
    if (!isTethered || foregroundPass != foreground || !before) {
        return;
    }
    const double fromX = d.drawnX(*this) - d.currentXcoord;
    const double fromY = d.drawnY(*this) + 30 - d.currentYcoord;
    const double toX = flightCircleX - d.currentXcoord;
    const double toY = flightCircleY + 30 - d.currentYcoord;
    d.drawScreenLine(fromX, fromY, toX, toY, 4, 2236962);
    d.drawScreenLine(fromX, fromY, toX, toY, 2, 10066329);
}

} // namespace papaGame::models::objects
