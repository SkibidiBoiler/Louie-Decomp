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
            onSlope == 0) {
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
    double slopeCheckDir = dy;
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
            slopeCheckDir = 1;
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
        if ((!HitDownLeft && !HitUpLeft && !HitUpMidLeft && !HitDownMidLeft) || (onSlope != 0 && !onSlopeWallLeft)) {
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
        if ((!HitUpRight && !HitDownRight && !HitUpMidRight && !HitDownMidRight) || (onSlope != 0 && !onSlopeWallRight)) {
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
    checkForSlopes(dx, slopeCheckDir);
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
        if (!HitDownLeft && !HitDownRight && !checkIfOnCloud() && !checkMovingTiles(1) && onSlope == 0 && !isInQuicksand) {
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

// AS: Character.checkForCeilingSlopes — pushes the head down out of a sloped ceiling.
void Character::checkForCeilingSlopes(double atY)
{
    const managers::MapManager &map = *gameObj->obf_4i;
    const double tw = gameObj->obf_71->tileWidth;
    if (!jump && atY <= -1) {
        return;
    }
    double head = y - playerheight;
    if (atY > -1) {
        head = atY - playerheight;
    }
    const double row = std::floor(head / tw);
    const double col = std::floor(x / tw);
    const double type = map.getTileProperty(col, row, "ceilingslope");
    if (type <= 0) {
        return;
    }
    const double ix = x - xtile * tw;
    double ceiling = head;
    if (type == 1) {
        ceiling = row * tw + ix;
    } else if (type == 2) {
        ceiling = row * tw + (tw - ix);
    } else if (type == 3) {
        ceiling = row * tw + std::floor(ix / 2);
    } else if (type == 4) {
        ceiling = row * tw + (tw / 2 + std::floor(ix / 2));
    } else if (type == 5) {
        ceiling = row * tw + (tw / 2 + std::floor((tw - ix) / 2));
    } else if (type == 6) {
        ceiling = row * tw + std::floor((tw - ix) / 2);
    } else if (type == 7 || type == 8) {
        ceiling = ix <= tw / 2 ? row * tw + ix * 2 : row * tw + (ix - tw / 2) * 2;
    } else if (type == 9 || type == 10) {
        ceiling = ix < tw / 2 ? row * tw + (tw - ix - tw / 2) * 2 : row * tw + std::floor((tw - ix) * 2);
    }
    if (ceiling > head) {
        const double headroom = y - head;
        y = std::ceil(ceiling + headroom + 2);
        ytile = std::floor(y / tw);
        if (jumpspeed < 0) {
            jumpspeed = 0;
        }
    }
}

// AS: Character.checkForSlopes — keeps the character on slope tiles (types 1-10: 45, 22 and 67 degree slopes,
// split in halves) and handles walking up into a sloped ceiling.
void Character::checkForSlopes(double dirX, double dirY)
{
    const managers::MapManager &map = *gameObj->obf_4i;
    const double tw = gameObj->obf_71->tileWidth;
    const char *prop = onlyCheckSolidSlopes ? "solidslope" : "slope";
    double rise = 0;
    const double savedY = y;
    const double here = map.getTileProperty(xtile, ytile, prop);
    const double below = map.getTileProperty(xtile, ytile + 1, prop);
    const double below2 = map.getTileProperty(xtile, ytile + 2, prop);
    // Snap down onto a slope just below (walking downhill), or up onto one just above.
    if (below != 0 && !jump && !isFlying && !onWall && !onCeiling) {
        if (below != 7 && below != 10) {
            ytile += 1;
            y = (ytile + 1) * tw - height;
            rise = 0;
        }
    } else if (here == 0 && below2 > 0 && !jump && (onSlope == 8 || onSlope == 9)) {
        ytile += 2;
        y = (ytile + 1) * tw - height;
        rise = 0;
    } else if (here == 0 && map.getTileProperty(xtile, ytile + 1, "collision") == 0 &&
               map.getTileProperty(xtile, ytile + 1, "thrublock") == 0 &&
               (map.getTileProperty(xtile, ytile + 2, "collision") > 0 || map.getTileProperty(xtile, ytile + 2, "thrublock") > 0) &&
               !jump && (onSlope == 8 || onSlope == 9)) {
        ytile += 1;
        y = (ytile + 1) * tw - height;
        rise = 0;
    } else if (map.getTileProperty(xtile, ytile - 1, prop) != 0 && !jump && !isFlying && !onWall && !onCeiling) {
        const double above = map.getTileProperty(xtile, ytile - 1, prop);
        if (above != 8 && above != 9) {
            --ytile;
            y = (ytile - 1) * tw - height;
            rise = 0;
        }
    }
    if (map.getTileProperty(xtile, ytile, prop) != 0 && dirY != -1) {
        if (isFlying) {
            diry = -1;
        }
        const double ix = std::floor(x - xtile * tw);
        onSlope = map.getTileProperty(xtile, ytile, prop);
        double target = -1; // y on the slope surface
        double slopeSide = 0;
        if (onSlope == 1) {
            rise = ix;
            target = (ytile + 1) * tw - height - rise;
            slopeSide = 1;
        } else if (onSlope == 2) {
            rise = tw - ix;
            target = (ytile + 1) * tw - height - rise;
            slopeSide = -1;
        } else if (onSlope == 3) {
            rise = ix;
            target = (ytile + 1) * tw - height - std::floor(rise / 2);
            slopeSide = 1;
        } else if (onSlope == 4) {
            rise = ix;
            target = (ytile + 1) * tw - height - (tw / 2 + std::floor(rise / 2));
            slopeSide = 1;
        } else if (onSlope == 5) {
            rise = tw - ix;
            target = (ytile + 1) * tw - height - (tw / 2 + std::floor(rise / 2));
            slopeSide = -1;
        } else if (onSlope == 6) {
            rise = tw - ix;
            target = (ytile + 1) * tw - height - std::floor(rise / 2);
            slopeSide = -1;
        } else if (onSlope == 7 || onSlope == 8) {
            double row = ytile;
            slopeSide = 1;
            if (ix <= tw / 2) {
                if (onSlope == 8) {
                    if (jump && jumpspeed < 0) {
                        target = (row + 1) * tw - height;
                        if (target > y) {
                            target = y;
                        }
                    } else {
                        row = ytile + 1;
                        rise = ix * 2;
                        target = (row + 1) * tw - height - std::floor(rise);
                    }
                } else {
                    rise = ix * 2;
                    target = (row + 1) * tw - height - std::floor(rise);
                }
            } else if (onSlope == 7) {
                row = ytile - 1;
                target = (row + 1) * tw - height - (ix - tw / 2) * 2;
            } else {
                target = (row + 1) * tw - height - (ix - tw / 2) * 2;
            }
        } else if (onSlope == 9 || onSlope == 10) {
            double row = ytile;
            slopeSide = -1;
            if (ix < tw / 2) {
                if (onSlope == 10) {
                    row = ytile - 1;
                }
                target = (row + 1) * tw - height - (tw - ix - tw / 2) * 2;
            } else if (onSlope == 9) {
                if (jump && jumpspeed < 0) {
                    target = (row + 1) * tw - height;
                    if (target > y) {
                        target = y;
                    }
                } else {
                    row = ytile + 1;
                    rise = (tw - ix) * 2;
                    target = (row + 1) * tw - height - std::floor(rise);
                }
            } else {
                rise = (tw - ix) * 2;
                target = (row + 1) * tw - height - std::floor(rise);
            }
        }
        if (target != -1) {
            if (jump && jumpspeed < 0 && slopeSide != dirX) {
                // Rising past a slope: only keep the head out of the ceiling.
                slopedIntoCeiling = 0;
                getMyCorners(x, target);
                if (HitUpLeft || HitUpRight || HitUpMidLeft || HitUpMidRight) {
                    const double minY = (HitUpMidLeft || HitUpMidRight) ? (upmidY + 1) * tw + playerheight + 1
                                                                       : (upY + 1) * tw + playerheight + 1;
                    if (y < minY) {
                        y = minY;
                    }
                    ytile = std::floor(y / tw);
                }
            } else if (jump && jumpspeed > 0 && y < target) {
                slopedIntoCeiling = 0; // still falling towards the slope
            } else {
                if (jump && jumpspeed < 0 && target > y) {
                    target = y;
                }
                getMyCorners(x, target);
                const bool blocked = (slopedIntoCeiling == 0 && (HitUpLeft || HitUpRight || HitUpMidLeft || HitUpMidRight)) ||
                                     (slopedIntoCeiling == 1 && (HitUpRight || HitUpMidRight)) ||
                                     (slopedIntoCeiling == -1 && (HitUpLeft || HitUpMidLeft));
                if (blocked) {
                    // Walking up the slope would put the head into the ceiling.
                    if (!jump || lastOnSlope != 0 || isGliding) {
                        x = lastx;
                        xtile = std::floor(x / tw);
                        y = lasty;
                        ytile = std::floor(y / tw);
                        onSlope = lastOnSlope;
                        jump = false;
                        isGliding = false;
                    } else if (jump) {
                        if (jumpspeed < 0) {
                            jumpspeed = 0;
                            if (isGliding) {
                                jump = false;
                                isGliding = false;
                                jumpspeed = 0;
                            }
                        }
                        if (jumpspeed <= 0) {
                            const double minY = (HitUpMidLeft || HitUpMidRight) ? (upmidY + 1) * tw + playerheight + 1
                                                                               : (upY + 1) * tw + playerheight + 1;
                            if (y < minY) {
                                y = minY;
                            }
                            onSlope = 0;
                        } else {
                            // "Jumping, but dropping": back to where the jump started if it was close.
                            const double dist = std::sqrt((y - jumpedFromY) * (y - jumpedFromY) + (x - jumpedFromX) * (x - jumpedFromX));
                            if (dist <= 32) {
                                x = jumpedFromX;
                                y = jumpedFromY;
                                onSlope = jumpedFromSlope;
                                jump = false;
                                xtile = std::floor(x / tw);
                                ytile = std::floor(y / tw);
                            } else {
                                onSlope = 0;
                            }
                        }
                        ytile = std::floor(y / tw);
                    }
                    if (checkForCrawlGap(dirX) && canCrawl && !isCrawling) {
                        // SKELETON: crawlgap.wav
                        isCrawling = true;
                        duck = true;
                        // isTurning lives in CustomerChar; crawling characters are not translated yet.
                    } else {
                        slopedIntoCeiling = slopeSide;
                        isWalking = false;
                    }
                } else {
                    y = target;
                    if (onSlope == 7 || onSlope == 8 || onSlope == 9 || onSlope == 10) {
                        ytile = std::floor(y / tw);
                    }
                    slopedIntoCeiling = 0;
                }
                if (onSlope != 0) {
                    if (jump && jumpspeed >= 0) {
                        // Landing on a slope.
                        if (bounceY) {
                            diry = -1;
                            jumpspeed = std::min(-10.0, 0 - std::fabs(jumpspeed * bounceDecayY));
                            speed = std::max(6.0, std::floor(speed * bounceDecayX));
                            ++numBounces;
                            if (onSlope == 1 || onSlope == 3 || onSlope == 4 || onSlope == 7 || onSlope == 8) {
                                dirx = -1;
                            } else if (onSlope == 2 || onSlope == 5 || onSlope == 6 || onSlope == 9 || onSlope == 10) {
                                dirx = 1;
                            }
                            if (jumpspeed == 0) {
                                dirx = 0;
                                jumpspeed = 0;
                                speed = 0;
                                bounceY = false;
                                jump = false;
                                triggerjump = false;
                                triggerjumpx = false;
                                isWallSliding = false;
                            }
                        } else {
                            jump = false;
                            triggerjump = false;
                            triggerjumpx = false;
                            isWallSliding = false;
                        }
                    }
                    slopeDir = slopeSide;
                }
            }
        }
        onSlopeWallRight = (map.getTileProperty(xtile + 1, ytile - 1, "collision") != 0 && onSlope != 7 && onSlope != 8) ||
                           (map.getTileProperty(xtile + 1, ytile - 2, "collision") != 0 && !duck);
        onSlopeWallLeft = (map.getTileProperty(xtile - 1, ytile - 1, "collision") != 0 && onSlope != 10 && onSlope != 9) ||
                          (map.getTileProperty(xtile - 1, ytile - 2, "collision") != 0 && !duck);
    } else {
        if (onSlope != 0) {
            // Leaving a slope: step onto the flat tile at the top or the bottom.
            if ((onSlope == 1 && dirX == 1) || (onSlope == 2 && dirX == -1) || (onSlope == 4 && dirX == 1) ||
                (onSlope == 5 && dirX == -1) || ((onSlope == 7 || onSlope == 8) && dirX == 1) ||
                ((onSlope == 10 || onSlope == 9) && dirX == -1)) {
                if (map.getTileProperty(xtile, ytile, "collision") != 0) {
                    --ytile;
                }
                y = (ytile + 1) * tw - height;
                if (isSliding) {
                    jump = true;
                    jumpspeed = -12;
                }
            } else if ((onSlope == 1 && dirX == -1) || (onSlope == 2 && dirX == 1) || (onSlope == 3 && dirX == -1) ||
                       (onSlope == 6 && dirX == 1) || ((onSlope == 7 || onSlope == 8) && dirX == -1) ||
                       ((onSlope == 10 || onSlope == 9) && dirX == 1)) {
                y = (ytile + 1) * tw - height;
            }
        }
        onSlope = 0;
        slopeDir = 0;
        slopedIntoCeiling = 0;
        rise = 0;
    }
    if (isFlying) {
        y = savedY;
        if (dirx == 1) {
            if (onSlope == 1 || onSlope == 3 || onSlope == 4 || onSlope == 7 || onSlope == 8) {
                dirx = -1;
            }
        } else if (dirx == -1) {
            if (onSlope == 2 || onSlope == 5 || onSlope == 6 || onSlope == 9 || onSlope == 10) {
                dirx = 1;
            }
        }
    }
    (void)rise;
}

} // namespace papaGame::models
