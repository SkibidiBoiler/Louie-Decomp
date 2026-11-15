#include "game/papaGame/models/Character.h"

#include <algorithm>
#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models {

// AS: Character.checkIfOnCloud
bool Character::checkIfOnCloud()
{
    const managers::MapManager &map = *gameObj->obf_4i;
    const double left = map.getTileProperty(leftX, downY, "thrublock");
    const double right = map.getTileProperty(rightX, downY, "thrublock");
    return (left == 1 || right == 1) && ytile != downY && (!isClimbingLadder || !checkIfLadderBelow());
}

// AS: Character.checkIfLadderBelow
bool Character::checkIfLadderBelow()
{
    return gameObj->obf_4i->getTileProperty(xtile, downY + 1, "ladder") > 0 && !jump;
}

// AS: Character.checkIfHalfOnCloud
bool Character::checkIfHalfOnCloud(double dir)
{
    const managers::MapManager &map = *gameObj->obf_4i;
    const double left = map.getTileProperty(leftX, downY, "thrublock");
    const double right = map.getTileProperty(rightX, downY, "thrublock");
    if (dir == -1 && left == 1 && ytile != downY) {
        return true;
    }
    return dir == 1 && right == 1 && ytile != downY;
}

// AS: Character.checkMovingTiles
bool Character::checkMovingTiles(double)
{
    // SKELETON: moving tiles (whichMovingTile.checkStandingOnObject) come with the objects.
    return false;
}

// AS: Character.checkIfAtEdge
void Character::checkIfAtEdge(double, double dir)
{
    const managers::MapManager &map = *gameObj->obf_4i;
    getMyCorners(x, y + 1);
    if (isWalking && !jump) {
        if ((!HitDownLeft || dir == 1) && (!HitDownRight || dir == -1) && !checkIfHalfOnCloud(dir) && !checkMovingTiles(1) &&
            !onSlope) {
            if (map.getTileProperty(xtile + dir, ytile + 1, "slope") == 0 && map.getTileProperty(xtile + dir, ytile, "slope") == 0) {
                dirx = dir * -1;
            }
        }
    }
}

// AS: Character.moveChar
bool Character::moveChar(double dx, double dy, double jumpDir, bool bounceAtWalls, bool checkEdges, bool collide,
                         double fixedSpeed)
{
    const display::GameDisplay &display = *gameObj->obf_71;
    const double tw = display.tileWidth;
    double slopeDir = dy;
    bool landed = false;
    double step = jumpDir != 0 ? jumpspeed * jumpDir : speed;
    if (fixedSpeed < INT_MAX) {
        step = fixedSpeed;
    }
    // SKELETON: flight wave (isFlying && useFlightWave) is for flying enemies.
    if (!collide) {
        if (dy != 0) {
            y += step * dy;
        }
        if (dx != 0) {
            x += step * dx;
        }
        xtile = std::floor(x / tw);
        ytile = std::floor(y / tw);
        return landed;
    }
    getMyCorners(x, y + step * dy);
    if (dy == -1) {
        if (!HitUpLeft && !HitUpRight) {
            y += step * dy;
        } else {
            y = (upY + 1) * tw + playerheight;
            jumpspeed = 0;
            slopeDir = 1;
            if (isFlying) {
                diry = 1;
            }
        }
    }
    if (dy == 1) {
        if ((!HitDownLeft || (SlopeDownRight > 0 && SlopeLeft > 0)) && (!HitDownRight || (SlopeDownLeft > 0 && SlopeRight > 0)) &&
            !checkIfOnCloud() && !checkMovingTiles(y)) {
            y += step * dy;
        } else {
            landed = true;
            const bool wasJumping = jump;
            const double oldJumpspeed = jumpspeed;
            if (bounceY) {
                jumpspeed = std::floor(std::fabs(jumpspeed * bounceDecayY) * -1);
                if (jumpspeed >= -2) {
                    jump = false;
                }
                speed = std::floor(speed * bounceDecayX);
                ++numBounces;
            } else {
                jump = false;
                triggerjump = false;
                triggerjumpx = false;
            }
            if (isFlying) {
                diry = -1;
            }
            if (!onMovingTile) {
                getMyCorners(x, (ytile + 1) * tw - height);
                if (HitUpLeft || HitUpRight || HitUpMidLeft || HitUpMidRight) {
                    y += step * dy;
                    if (wasJumping) {
                        jump = true;
                        jumpspeed = oldJumpspeed;
                    }
                } else {
                    y = (ytile + 1) * tw - height;
                    ytile = std::floor(y / tw);
                    if (isClimbingLadder) {
                        isClimbingLadder = false;
                    }
                }
            }
        }
    }
    getMyCorners(x + step * dx, y);
    if ((dx == -1 && step > 0) || (dx == 1 && step < 0)) {
        if ((!HitDownLeft && !HitUpLeft && !HitUpMidLeft && !HitDownMidLeft) || (onSlope && !onSlopeWallLeft)) {
            x += step * dx;
            if (!(!isWalking && fixedSpeed < INT_MAX && onMovingTile)) {
                isWalking = true;
            }
        } else if (checkForCrawlGap(-1)) {
            // SKELETON: crawlgap.wav
            x += step * dx;
            isWalking = true;
            isCrawling = true;
        } else {
            x = xtile * tw + width;
            isSliding = false;
            if (bounceAtWalls) {
                dirx = 1;
            } else {
                isWalking = false;
                if (bounceX) {
                    speed = std::floor(speed * bounceDecayX);
                    dirx = 1;
                    ++numBounces;
                    isWalking = true;
                } else {
                    dirx = 0;
                }
            }
        }
    }
    if ((dx == 1 && step > 0) || (dx == -1 && step < 0)) {
        if ((!HitUpRight && !HitDownRight && !HitUpMidRight && !HitDownMidRight) || (onSlope && !onSlopeWallRight)) {
            x += step * dx;
            if (!(!isWalking && fixedSpeed < INT_MAX && onMovingTile)) {
                isWalking = true;
            }
        } else if (checkForCrawlGap(1)) {
            x += step * dx;
            isWalking = true;
            isCrawling = true;
        } else {
            x = (xtile + 1 + (widthmultiplier - 1) / 2) * tw - width * widthmultiplier;
            isSliding = false;
            if (bounceAtWalls) {
                dirx = -1;
            } else {
                isWalking = false;
                if (bounceX) {
                    speed = std::floor(speed * bounceDecayX);
                    dirx = -1;
                    ++numBounces;
                    isWalking = true;
                } else {
                    dirx = 0;
                }
            }
        }
    }
    const auto &tiles = gameObj->obf_AQ->currentScreenData->tileArray;
    const double minX = 0, maxX = static_cast<double>(tiles[0].size()) * tw;
    const double minY = 0, maxY = static_cast<double>(tiles.size()) * tw;
    if (x < minX) {
        x = minX;
        dirx = 1;
        if (isPlayer()) {
            checkGoingThroughDoor(-1, 0);
        }
    }
    if (isPlayer() && gameObj->obf_3Y->lockedCamera) {
        if (x < display.currentXcoord + 16) {
            x = display.currentXcoord + 16;
        } else if (x > display.currentXcoord + display.screenPxWidth - 16) {
            x = display.currentXcoord + display.screenPxWidth - 16;
        }
    }
    if (x > maxX) {
        if (isPlayer() && isFinishingLevelPlayer()) {
            // SKELETON: walking off the end of the level starts the "out" transition.
            if (x > maxX + spritewidth / 2) {
                x = maxX + spritewidth / 2;
                isWalking = false;
            }
        } else {
            x = maxX;
            dirx = -1;
            if (isPlayer()) {
                checkGoingThroughDoor(1, 0);
            }
        }
    }
    if (y < minY) {
        y = minY;
        if (onWall) {
            canClimbUp = false;
        }
        if (isFlying) {
            diry = 1;
        }
        if (isPlayer()) {
            checkGoingThroughDoor(0, -1);
        }
    }
    if (y > maxY) {
        if (isPlayer() && !isFinishingLevelPlayer() && y > maxY + heightmultiplier * height) {
            y = maxY + heightmultiplier * height - 3;
            fellOffLevel(); // restartPlayer / hurtPlayer, then checkGoingThroughDoor(0, 1)
        }
        if (y > maxY + spriteheight + 50) {
            y = maxY + spriteheight + 50;
        }
        if (isFlying) {
            diry = -1;
        }
    }
    xtile = std::floor(x / tw);
    ytile = std::floor(y / tw);
    if (shouldCheckCeilingSlopes) {
        checkForCeilingSlopes();
    }
    checkForSlopes(dx, slopeDir);
    if (checkEdges) {
        checkIfAtEdge(step, dx);
    }
    return landed;
}

// AS: Character.fallChar
void Character::fallChar()
{
    if (!jump && !isFlying) {
        getMyCorners(x, y + 1);
        if (!HitDownLeft && !HitDownRight && !checkIfOnCloud() && !checkMovingTiles(1) && !onSlope && !isInQuicksand) {
            jumpspeed = 0;
            jump = true;
            jumpedFromX = x;
            jumpedFromY = y;
        }
    }
}

// AS: Character.jumpChar
void Character::jumpChar(bool collide)
{
    const double gameplayTimer = 0; // SKELETON: GameControls.gameplayTimer (only used while gliding / wall sliding)
    if (isWallSliding) {
        if (std::fmod(gameplayTimer, 10) == 0) {
            jumpspeed += gravity;
        }
    } else if (!isGliding || std::fmod(gameplayTimer, 4) == 0) {
        jumpspeed += gravity;
    }
    // SKELETON: carried objects add their grabWeight.
    double maxFall = std::min(maxjumpvelocity, gameObj->obf_71->tileWidth - height);
    if (isGliding) {
        maxFall = 2;
    }
    if (jumpspeed > maxFall) {
        jumpspeed = maxFall;
    }
    if (jumpspeed < 0) {
        moveChar(0, -1, -1, false, false, collide);
    } else if (jumpspeed > 0) {
        if (!isGliding) {
            gravity = normalgravity;
        }
        moveChar(0, 1, 1, false, false, collide);
    }
}

// AS: Character.getMyCorners
void Character::getMyCorners(double px, double py)
{
    const double tw = gameObj->obf_71->tileWidth;
    const managers::MapManager &map = *gameObj->obf_4i;
    downY = std::floor((py + height - 1) / tw);
    upY = std::floor((py - height) / tw);
    if (duck && (canCrawl || onMovingTile)) {
        upY = std::floor((py - height) / tw);
        standupY = std::floor((py - playerheight) / tw);
        if (standupY < upY && upY - standupY > 1) {
            standupmidY = upY - 1;
        }
    } else {
        upY = std::floor((py - playerheight) / tw);
        standupY = upY;
        standupmidY = upY;
    }
    leftX = std::floor((px - width) / tw);
    rightX = std::floor((px + width * widthmultiplier - 1) / tw);
    if (downY != upY) {
        midY = upY + (downY - upY - 1);
        downmidY = midY;
        upmidY = downY - (downY - upY - 1);
    } else {
        midY = downY;
        downmidY = midY;
        upmidY = midY;
    }
    if (upmidY >= downY) {
        upmidY = upY;
    }
    if (downmidY > downY) {
        downmidY = downY;
    }
    HitUpLeft = map.getTileProperty(leftX, upY, "collision") != 0;
    HitDownLeft = map.getTileProperty(leftX, downY, "collision") != 0;
    HitUpRight = map.getTileProperty(rightX, upY, "collision") != 0;
    HitDownRight = map.getTileProperty(rightX, downY, "collision") != 0;
    HitMidLeft = map.getTileProperty(leftX, midY, "collision") != 0;
    HitMidRight = map.getTileProperty(rightX, midY, "collision") != 0;
    HitUpMidLeft = map.getTileProperty(leftX, upmidY, "collision") != 0;
    HitUpMidRight = map.getTileProperty(rightX, upmidY, "collision") != 0;
    HitDownMidLeft = HitMidLeft;
    HitDownMidRight = HitMidRight;
    HitStandUpLeft = map.getTileProperty(leftX, standupY, "collision") != 0;
    HitStandUpRight = map.getTileProperty(rightX, standupY, "collision") != 0;
    SlopeDownLeft = map.getTileProperty(leftX, downY, "slope");
    SlopeDownRight = map.getTileProperty(rightX, downY, "slope");
    SlopeLeft = map.getTileProperty(leftX, downY - 1, "slope");
    SlopeRight = map.getTileProperty(rightX, downY - 1, "slope");
}

// AS: Character.checkForCeilingSlopes
void Character::checkForCeilingSlopes(double)
{
    // SKELETON: not translated yet (ceiling slopes).
}

// AS: Character.checkForSlopes
void Character::checkForSlopes(double, double)
{
    // SKELETON: not translated yet; onSlope stays false, so slopes act as empty tiles for now.
}

} // namespace papaGame::models
