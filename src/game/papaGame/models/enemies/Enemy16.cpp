#include "game/papaGame/models/enemies/Enemy16.h"

#include <cmath>

#include "game/obf_AF/obf_1n.h"
#include "game/obf_AF/obf_8J.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

using obf_AF::obf_1n;

// AS: Enemy16.Enemy16 — it starts on the surface next to it.
void Enemy16::init(double xtile_, double ytile_, double id_, double whichScreen_, double whichTileset_,
                   double screenObjID_, const std::vector<as3::Value> &extraParams)
{
    Enemy::init(xtile_, ytile_, id_, whichScreen_, whichTileset_, screenObjID_, extraParams);
    findStartupSurface();
}

// AS: Enemy16.defineVars
void Enemy16::defineVars()
{
    type = 16;
    sheetname = "enemysheet_dillwheel";
    sheetIsBitmap = false;
    height = 15;
    width = 15;
    playerheight = 15;
    spritewidth = 50;
    spriteheight = 50;
    spriteCenterX = 25;
    spriteCenterY = 25;
    spriteTargetX = 25;
    spriteTargetY = 25;
    sheetWidth = 30;
    sheetHeight = 2;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 4;
    walkspeed = 4;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -16;
    flipBlitOffset = 0;
    collRect = {-9, -9, 18, 18};
    gravity = 1;
    normalgravity = 1;
    currentHealth = 1;
    maxHealth = 1;
    facingWalking = true;
    points = 10;
    canBeStunned = true;
    shouldCheckCeilingSlopes = true;
    enemyCategory = "dill";
}

// AS: Enemy16.setupParams — [direction, rememberKilled]
void Enemy16::setupParams(const std::vector<as3::Value> &params)
{
    if (params.size() > 0) {
        facingDir = params[0].asNumber();
        walkingDir = facingDir;
        dirx = walkingDir;
        originalDirX = dirx;
    }
    if (params.size() > 1) {
        rememberKilled = params[1].asNumber() == 1;
    }
}

// AS: Enemy16.stunEnemy — a stun is a hit, away from the player.
void Enemy16::stunEnemy()
{
    double dir = 1;
    if (gameObj->playerObj->x > x) {
        dir = -1;
    }
    getHit(1, dir, false);
}

// AS: Enemy16.grabAnimationCycle — it spins the way it first rolled.
const AnimationCycle &Enemy16::grabAnimationCycle()
{
    if (isDead) {
        return cycleFrames("dieCycleFrames");
    }
    if (originalDirX == 1) {
        return cycleFrames("rollRightCycleFrames");
    }
    return cycleFrames("rollLeftCycleFrames");
}

// AS: Enemy16.checkCloudTile — a one-way tile here, or (below) one under an open tile.
bool Enemy16::checkCloudTile(double tx, double ty, bool below) const
{
    const managers::MapManager &map = *gameObj->obf_4i;
    const bool here = map.getTileProperty(tx, ty, "thrublock") != 0;
    const bool under = map.getTileProperty(tx, ty + 1, "thrublock") != 0;
    if (below == false && here == true) {
        return true;
    }
    if (below == true && here == false && under == true) {
        return true;
    }
    return false;
}

// AS: Enemy16.checkForCollision — a solid tile, or a one-way one (below: the one under it).
bool Enemy16::checkForCollision(double tx, double ty, bool below) const
{
    const bool solid = gameObj->obf_4i->getTileProperty(tx, ty, "collision") != 0;
    bool cloud = checkCloudTile(tx, ty);
    if (below) {
        cloud = checkCloudTile(tx, ty + 1);
    }
    return solid || cloud;
}

// AS: Enemy16.checkForSlope
bool Enemy16::checkForSlope(double tx, double ty) const
{
    return gameObj->obf_4i->getTileProperty(tx, ty, "slope") > 0;
}

// AS: Enemy16.checkForCeilingSlope — on the ceiling it looks for the reversed ones.
bool Enemy16::checkForCeilingSlope(double tx, double ty) const
{
    const managers::MapManager &map = *gameObj->obf_4i;
    double slope = map.getTileProperty(tx, ty, "ceilingslope");
    if (onCeiling) {
        slope = map.getTileProperty(tx, ty, "ceilingslopereverse");
    }
    return slope > 0;
}

// AS: Enemy16.updateObject — it rolls along its surface (or eases round a corner); it keeps rolling while any dill
// wheel is on screen.
void Enemy16::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    speed = adjustSpeedForSlopes(walkspeed);
    if (isDead) {
        onWall = false;
        onCeiling = false;
        onGround = false;
        jumpChar(false);
        moveChar(dirx, 0, 0, false, false, false);
        updateSprite();
        if (ytile > d.currentYtileDown + 1) {
            removeEnemy();
        } else if (isLaunching && (ytile <= d.currentYtileUp - 1 || ytile < 0)) {
            removeEnemy();
        }
    } else if (checkOnScreen() || gameObj->obf_E->checkEnemyTypeOnScreen(type)) {
        if (isTransitioning) {
            const double t = transTimer;
            const double duration = transitionDuration;
            x = obf_AF::obf_8J::obf_5o(t, transitionStartX, transitionTargetX - transitionStartX, duration);
            y = obf_AF::obf_8J::obf_5o(t, transitionStartY, transitionTargetY - transitionStartY, duration);
            if (t >= duration) {
                isTransitioning = false;
                transTimer = 0;
                x = transitionTargetX;
                y = transitionTargetY;
            }
            xtile = std::floor(x / d.tileWidth);
            ytile = std::floor(y / d.tileWidth);
            ++transTimer;
        } else {
            if (dirx != 0) {
                moveChar(dirx, 0, 0);
            } else if (diry != 0) {
                moveChar(0, diry, 0);
            }
            if (dirx != 0) {
                walkingDir = dirx;
            } else {
                walkingDir = diry;
            }
        }
        if (jump) {
            if (jumpspeed > 0) {
                diry = 1;
            } else if (jumpspeed < 0) {
                diry = -1;
            } else if (jumpspeed == 0) {
                diry = 0;
            }
            jumpChar();
        }
        if (!jump && !onGround && !onCeiling && !onWall && !isTransitioning) {
            onGround = true;
        }
        facingDir = dirx;
        walkingDir = dirx;
        updateSprite();
        lastx = x;
        lasty = y;
        lastOnSlope = onSlope;
    }
}

// AS: Enemy16.moveChar — Character.moveChar reworked to follow the surface: at the end of a floor, wall or ceiling
// it turns onto the next one (easing round outside corners with setupTransition).
bool Enemy16::moveChar(double dx, double dy, double jumpDir, bool /*bounceAtWalls*/, bool /*checkEdges*/,
                       bool collide, double fixedSpeed)
{
    const display::GameDisplay &d = *gameObj->obf_71;
    const double tw = d.tileWidth;
    double slopeDirY = dy;
    double step;
    if (jumpDir != 0) {
        step = jumpspeed * jumpDir;
    } else {
        step = speed;
    }
    if (fixedSpeed < INT_MAX) {
        step = fixedSpeed;
    }
    if (collide == false) {
        if (dy != 0) {
            y += step * dy;
        }
        if (dx != 0) {
            x += step * dx;
        }
        xtile = std::floor(x / tw);
        ytile = std::floor(y / tw);
        return true;
    }

    double cornerX = x;
    if (onWall) {
        cornerX += wallDir;
    }
    getMyCorners(cornerX, y + step * dy);
    if (dy == -1) {
        if (onWall) {
            const bool cloudLeft = checkCloudTile(leftX, ytile);
            const bool cloudRight = checkCloudTile(rightX, ytile);
            if (upY < 0) {
                diry = 1;
                y = width;
            } else if (((HitUpLeft || cloudLeft) && (HitUpRight || cloudRight)) || checkForCeilingSlope(xtile, ytile)) {
                // Up a wall into a ceiling: roll along it.
                if (wallDir == 1) {
                    dirx = -1;
                } else {
                    dirx = 1;
                }
                onCeiling = true;
                onWall = false;
                diry = 0;
                if (cloudLeft || cloudRight) {
                    setupTransition(x, upY * tw + playerheight);
                } else {
                    y = (upY + 1) * tw + playerheight;
                }
            } else if ((!HitUpLeft && wallDir == -1) || (!HitUpRight && wallDir == 1)) {
                // Over the top of a wall onto the floor beyond.
                setupTransition(x + wallDir * 6, ytile * tw - height);
                dirx = wallDir;
                onWall = false;
                onCeiling = false;
                onGround = true;
                diry = 0;
            } else {
                y += step * dy;
            }
        } else if (!HitUpLeft && !HitUpRight) {
            y += step * dy;
        } else {
            y = (upY + 1) * tw + playerheight;
            jumpspeed = 0;
            slopeDirY = 1;
        }
    }
    if (dy == 1) {
        if (onWall) {
            // AS: it also checks for one-way tiles left and right here, and never uses the answers.
            if (downY >= d.levelTileHeight) {
                diry = -1;
                y = d.levelPxHeight - width;
            } else if ((HitDownLeft && HitDownRight) ||
                       (checkCloudTile(xtile, downY) == true && checkCloudTile(xtile, upY) == false) ||
                       checkForSlope(xtile, ytile)) {
                // Down a wall onto the floor: roll along it.
                y = downY * tw - height;
                if (wallDir == 1) {
                    dirx = -1;
                } else {
                    dirx = 1;
                }
                onWall = false;
                onGround = true;
                diry = 0;
            } else if ((!HitDownLeft && wallDir == -1) || (!HitDownRight && wallDir == 1)) {
                // Off the bottom of a wall onto the ceiling beyond.
                setupTransition(x + wallDir * 6, downY * tw + height);
                dirx = wallDir;
                diry = 0;
                onWall = false;
                onGround = false;
                onCeiling = true;
            } else {
                y += step * dy;
            }
        } else if ((!HitDownLeft || (SlopeDownRight > 0 && SlopeLeft > 0)) &&
                   (!HitDownRight || (SlopeDownLeft > 0 && SlopeRight > 0)) && !checkIfOnCloud() &&
                   !checkMovingTiles(y)) {
            y += step * dy;
        } else {
            const bool wasJumping = jump;
            const double lastJumpSpeed = jumpspeed;
            jump = false;
            triggerjump = false;
            triggerjumpx = false;
            if (!onMovingTile) {
                getMyCorners(x, (ytile + 1) * tw - height);
                if (HitUpLeft || HitUpRight || HitUpMidLeft || HitUpMidRight) {
                    y += step * dy;
                    if (wasJumping) {
                        jump = true;
                        jumpspeed = lastJumpSpeed;
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
    if (dx == -1) {
        const bool floorAhead = checkForCollision(leftX, downY + 1);
        const bool ceilingAhead = checkForCollision(leftX, upY - 1, true);
        const bool slopeAhead = checkForSlope(leftX, downY + 1);
        const bool ceilingSlopeAhead = checkForCeilingSlope(leftX, upY - 1);
        const bool slopeBeside = checkForSlope(leftX, ytile);
        const bool ceilingSlopeBeside = checkForCeilingSlope(leftX, ytile);
        if (onGround) {
            if ((!HitDownLeft && !HitUpLeft && !HitUpMidLeft && !HitDownMidLeft) || (onSlope != 0 && !onSlopeWallLeft)) {
                x += step * dx;
                if (leftX < 0) {
                    dirx = 1;
                    x = width;
                } else if (!floorAhead && onSlope == 0 && !slopeBeside && !slopeAhead) {
                    // Off the end of the floor and down its side.
                    setupTransition((leftX + 1) * tw - width, y + 6);
                    onGround = false;
                    onWall = true;
                    wallDir = 1;
                    dirx = 0;
                    diry = 1;
                }
            } else if (HitDownLeft && HitUpLeft) {
                // Into a wall: climb it.
                x = xtile * tw + width;
                onGround = false;
                onWall = true;
                wallDir = -1;
                dirx = 0;
                diry = -1;
            } else {
                x = xtile * tw + width;
            }
        } else if (onCeiling) {
            if ((!HitDownLeft && !HitUpLeft && !HitDownMidLeft && !HitUpMidLeft) ||
                (onCeilingSlope != 0 && !onCeilingSlopeWallLeft)) {
                x += step * dx;
                if (leftX < 0) {
                    dirx = 1;
                    x = width;
                } else if (onCeilingSlope == 0) {
                    if (!ceilingAhead && !ceilingSlopeAhead && !ceilingSlopeBeside && !checkCloudTile(leftX, ytile)) {
                        // Off the end of the ceiling and up its side.
                        setupTransition(xtile * tw - width, y - 6);
                        onGround = false;
                        onCeiling = false;
                        onWall = true;
                        wallDir = 1;
                        dirx = 0;
                        diry = -1;
                    }
                }
            } else if (HitDownLeft && HitUpLeft) {
                // Into a wall: down it.
                x = xtile * tw + width;
                onCeiling = false;
                onWall = true;
                wallDir = -1;
                dirx = 0;
                diry = 1;
            } else {
                x = xtile * tw + width;
            }
        } else if ((!HitDownLeft && !HitUpLeft && !HitUpMidLeft && !HitDownMidLeft) || (onSlope != 0 && !onSlopeWallLeft)) {
            x += step * dx;
            if (!(isWalking == false && fixedSpeed < INT_MAX && onMovingTile)) {
                isWalking = true;
            }
        } else {
            x = xtile * tw + width;
            isWalking = false;
            dirx = 0;
        }
    }
    if (dx == 1) {
        const bool floorAhead = checkForCollision(rightX, downY + 1);
        const bool ceilingAhead = checkForCollision(rightX, upY - 1, true);
        const bool slopeAhead = checkForSlope(rightX, downY + 1);
        const bool ceilingSlopeAhead = checkForCeilingSlope(rightX, upY - 1);
        const bool slopeBeside = checkForSlope(rightX, ytile);
        const bool ceilingSlopeBeside = checkForCeilingSlope(rightX, ytile);
        if (onGround) {
            if ((!HitUpRight && !HitDownRight && !HitUpMidRight && !HitDownMidRight) || (onSlope != 0 && !onSlopeWallRight)) {
                x += step * dx;
                if (rightX >= d.levelTileWidth) {
                    dirx = -1;
                    x = d.levelPxWidth - width;
                } else if (!floorAhead && onSlope == 0 && !slopeBeside && !slopeAhead) {
                    setupTransition(rightX * tw + width, y + 6);
                    onGround = false;
                    onWall = true;
                    wallDir = -1;
                    dirx = 0;
                    diry = 1;
                }
            } else if (HitDownRight && HitUpRight) {
                x = (xtile + 1) * tw - width;
                onGround = false;
                onWall = true;
                wallDir = 1;
                dirx = 0;
                diry = -1;
            } else {
                x = (xtile + 1) * tw - width;
            }
        } else if (onCeiling) {
            if (rightX >= d.levelTileWidth) {
                dirx = -1;
                x = d.levelPxWidth - width;
            } else if ((!HitUpRight && !HitDownRight && !HitUpMidRight && !HitDownMidRight) ||
                       (onCeilingSlope != 0 && !onCeilingSlopeWallRight)) {
                x += step * dx;
                if (onCeilingSlope == 0) {
                    if (!ceilingAhead && !ceilingSlopeAhead && !ceilingSlopeBeside && !checkCloudTile(rightX, ytile)) {
                        setupTransition(rightX * tw + width, y - 6);
                        onGround = false;
                        onCeiling = false;
                        onWall = true;
                        wallDir = -1;
                        dirx = 0;
                        diry = -1;
                    }
                }
            } else if (HitDownRight && HitUpRight) {
                x = (xtile + 1) * tw - width;
                onCeiling = false;
                onWall = true;
                wallDir = 1;
                dirx = 0;
                diry = 1;
            } else {
                x = (xtile + 1) * tw - width;
            }
        } else if ((!HitUpRight && !HitDownRight && !HitUpMidRight && !HitDownMidRight) || (onSlope != 0 && !onSlopeWallRight)) {
            x += step * dx;
            if (!(isWalking == false && fixedSpeed < INT_MAX && onMovingTile)) {
                isWalking = true;
            }
        } else {
            x = (xtile + 1 + (widthmultiplier - 1) / 2) * tw - width * widthmultiplier;
            isWalking = false;
            dirx = 0;
        }
    }

    const auto &tiles = gameObj->obf_AQ->currentScreenData->tileArray;
    const double minX = 0;
    const double maxX = static_cast<double>(tiles.empty() ? 0 : tiles[0].size()) * d.tileWidth;
    const double minY = 0;
    const double maxY = static_cast<double>(tiles.size()) * d.tileWidth;
    if (x < minX) {
        x = minX;
        dirx = 1;
    }
    if (x > maxX) {
        x = maxX;
        dirx = -1;
    }
    if (y < minY) {
        y = minY;
        if (onWall) {
            canClimbUp = false;
        }
    }
    if (y > maxY) {
        if (y > maxY + spriteheight + 50) {
            y = maxY + spriteheight + 50;
        }
    }
    xtile = std::floor(x / tw);
    ytile = std::floor(y / tw);
    if (onCeiling) {
        checkForCeilSlopes(dx, -1);
    } else {
        onCeilingSlope = 0;
    }
    if (!onWall && !onCeiling) {
        checkForSlopes(dx, slopeDirY);
    } else {
        onSlope = 0;
    }
    return true;
}

// AS: Enemy16.findStartupSurface — floor, ceiling, right wall, left wall, or it falls. On a ceiling (or rolling down
// a wall) it spins the other way.
void Enemy16::findStartupSurface()
{
    const double tw = gameObj->obf_71->tileWidth;
    const bool below = checkForCollision(xtile, ytile + 1);
    const bool above = checkForCollision(xtile, ytile - 1, true);
    const bool right = checkForCollision(xtile + 1, ytile);
    const bool left = checkForCollision(xtile - 1, ytile);
    jump = false;
    if (below) {
        onGround = true;
        diry = 0;
        y = (ytile + 1) * tw - height;
    } else if (above) {
        onCeiling = true;
        diry = 0;
        y = ytile * tw + height;
        originalDirX *= -1;
    } else if (right) {
        onWall = true;
        wallDir = 1;
        if (dirx == 1) {
            diry = -1;
            dirx = 0;
        } else {
            diry = 1;
            dirx = 0;
            originalDirX *= -1;
        }
        x = (xtile + 1) * tw - height;
    } else if (left) {
        onWall = true;
        wallDir = -1;
        if (dirx == 1) {
            diry = 1;
            dirx = 0;
            originalDirX *= -1;
        } else {
            diry = -1;
            dirx = 0;
        }
        x = xtile * tw + height;
    } else {
        fallChar();
    }
}

// AS: Enemy16.checkForCeilSlopes — Character.checkForSlopes for the reversed ceiling slopes: it follows their line
// and, past the end of one, turns up the wall.
void Enemy16::checkForCeilSlopes(double dx, double /*dy*/)
{
    const managers::MapManager &map = *gameObj->obf_4i;
    const double tw = gameObj->obf_71->tileWidth;
    const double here = map.getTileProperty(xtile, ytile, "ceilingslopereverse");
    const double above = map.getTileProperty(xtile, ytile - 1, "ceilingslopereverse");
    const double above2 = map.getTileProperty(xtile, ytile - 2, "ceilingslopereverse");
    if (above != 0) {
        if (above != 7 && above != 10) {
            --ytile;
            y = ytile * tw + height;
        }
    } else if (here == 0 && above2 > 0 && (onCeilingSlope == 8 || onCeilingSlope == 9)) {
        ytile -= 2;
        y = ytile * tw + height;
    } else if (here == 0 &&
               (map.getTileProperty(xtile, ytile - 1, "collision") == 0 &&
                map.getTileProperty(xtile, ytile - 1, "thrublock") == 0) &&
               (map.getTileProperty(xtile, ytile - 2, "collision") > 0 ||
                map.getTileProperty(xtile, ytile - 2, "thrublock") > 0) &&
               (onCeilingSlope == 8 || onCeilingSlope == 9)) {
        --ytile;
        y = ytile * tw + height;
    } else if (map.getTileProperty(xtile, ytile + 1, "ceilingslopereverse") != 0) {
        const double below = map.getTileProperty(xtile, ytile + 1, "ceilingslopereverse");
        if (below != 8 && below != 9) {
            ytile += 1;
            y = ytile * tw + height;
        }
    }
    if (map.getTileProperty(xtile, ytile, "ceilingslopereverse") != 0) {
        const double inTile = std::floor(x - xtile * tw);
        onCeilingSlope = map.getTileProperty(xtile, ytile, "ceilingslopereverse");
        double targetY = -1;
        double offset = 0;
        if (!onMovingTile) {
            if (onCeilingSlope == 1) {
                offset = inTile;
                targetY = ytile * tw + height + offset;
            } else if (onCeilingSlope == 2) {
                offset = tw - inTile;
                targetY = ytile * tw + height + offset;
            } else if (onCeilingSlope == 3) {
                offset = inTile;
                targetY = ytile * tw + height + std::floor(offset / 2);
            } else if (onCeilingSlope == 4) {
                offset = inTile;
                targetY = ytile * tw + height + (tw / 2 + std::floor(offset / 2));
            } else if (onCeilingSlope == 5) {
                offset = tw - inTile;
                targetY = ytile * tw + height + (tw / 2 + std::floor(offset / 2));
            } else if (onCeilingSlope == 6) {
                offset = tw - inTile;
                targetY = ytile * tw + height + std::floor(offset / 2);
            } else if (onCeilingSlope == 7 || onCeilingSlope == 8) {
                double row = ytile;
                if (inTile <= tw / 2) {
                    if (onCeilingSlope == 8) {
                        row = ytile - 1;
                        offset = inTile * 2;
                        targetY = row * tw + height + std::floor(offset);
                    } else if (onCeilingSlope == 7) {
                        offset = inTile * 2;
                        targetY = row * tw + height + std::floor(offset);
                    }
                } else if (onCeilingSlope == 7) {
                    row = ytile + 1;
                    targetY = row * tw + height + (inTile - tw / 2) * 2;
                } else if (onCeilingSlope == 8) {
                    targetY = row * tw + height + (inTile - tw / 2) * 2;
                }
            } else if (onCeilingSlope == 9 || onCeilingSlope == 10) {
                double row = ytile;
                if (inTile < tw / 2) {
                    if (onCeilingSlope == 10) {
                        row = ytile + 1;
                        targetY = row * tw + height + (tw - inTile - tw / 2) * 2;
                    } else if (onCeilingSlope == 9) {
                        targetY = row * tw + height + (tw - inTile - tw / 2) * 2;
                    }
                } else if (onCeilingSlope == 9) {
                    row = ytile - 1;
                    offset = (tw - inTile) * 2;
                    targetY = row * tw + height + std::floor(offset);
                } else if (onCeilingSlope == 10) {
                    offset = (tw - inTile) * 2;
                    targetY = row * tw + height + std::floor(offset);
                }
            }
        }
        if (targetY != -1) {
            getMyCorners(x, targetY);
            y = targetY;
            if (onCeilingSlope == 7 || onCeilingSlope == 8 || onCeilingSlope == 9 || onCeilingSlope == 10) {
                ytile = std::floor(y / tw);
            }
        }
        onCeilingSlopeWallRight =
            (map.getTileProperty(xtile + 1, ytile + 1, "collision") != 0 && onCeilingSlope != 7 && onCeilingSlope != 8) ||
            map.getTileProperty(xtile + 1, ytile + 2, "collision") != 0;
        onCeilingSlopeWallLeft =
            (map.getTileProperty(xtile - 1, ytile + 1, "collision") != 0 && onCeilingSlope != 10 && onCeilingSlope != 9) ||
            map.getTileProperty(xtile - 1, ytile + 2, "collision") != 0;
    } else {
        if (onCeilingSlope != 0) {
            const double s = onCeilingSlope;
            if ((s == 1 && dx == 1) || (s == 2 && dx == -1) || (s == 4 && dx == 1) || (s == 5 && dx == -1) ||
                ((s == 7 || s == 8) && dx == 1) || ((s == 10 || s == 9) && dx == -1)) {
                if (map.getTileProperty(xtile, ytile, "collision") != 0) {
                    ytile += 1;
                }
                y = ytile * tw + height;
            } else if ((s == 1 && dx == -1) || (s == 2 && dx == 1) || (s == 3 && dx == -1) || (s == 6 && dx == 1) ||
                       ((s == 7 || s == 8) && dx == -1) || ((s == 10 || s == 9) && dx == 1)) {
                y = ytile * tw + height;
                if (map.getTileProperty(xtile, ytile - 1, "collision") == 0 &&
                    map.getTileProperty(xtile, ytile, "thrublock") == 0) {
                    if (dx == 1) {
                        setupTransition(xtile * tw + width, y - 6);
                        onGround = false;
                        onCeiling = false;
                        onWall = true;
                        wallDir = -1;
                        dirx = 0;
                        diry = -1;
                    } else if (dx == -1) {
                        setupTransition((xtile + 1) * tw - width, y - 6);
                        onGround = false;
                        onCeiling = false;
                        onWall = true;
                        wallDir = 1;
                        dirx = 0;
                        diry = -1;
                    }
                }
            }
        }
        onCeilingSlope = 0;
    }
}

// AS: Enemy16.adjustSpeedForSlopes — Enemy's, for ceiling slopes too.
double Enemy16::adjustSpeedForSlopes(double s) const
{
    if (onSlope == 1 || onSlope == 2 || onCeilingSlope == 1 || onCeilingSlope == 2) {
        return s * std::cos(obf_1n::degreesToRads(45));
    }
    if (onSlope == 3 || onSlope == 4 || onSlope == 5 || onSlope == 6 || onCeilingSlope == 3 || onCeilingSlope == 4 ||
        onCeilingSlope == 5 || onCeilingSlope == 6) {
        return s * std::cos(obf_1n::degreesToRads(22));
    }
    if (onSlope == 7 || onSlope == 8 || onCeilingSlope == 7 || onCeilingSlope == 8) {
        return s * std::cos(obf_1n::degreesToRads(67));
    }
    if (onSlope == 9 || onSlope == 10 || onCeilingSlope == 9 || onCeilingSlope == 10) {
        return s * std::cos(obf_1n::degreesToRads(67));
    }
    return s;
}

// AS: Enemy16.checkOnScreen — two tiles round the screen, up and down too.
bool Enemy16::checkOnScreen()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 2 && xtile < d.currentXtileRight + 2 && ytile > d.currentYtileUp - 2 &&
           ytile < d.currentYtileDown + 2;
}

// AS: Enemy16.setupTransition — ease to (targetX, targetY) at walking speed.
void Enemy16::setupTransition(double targetX, double targetY)
{
    isTransitioning = true;
    startTransition = 0;
    transitionStartX = x;
    transitionStartY = y;
    transitionTargetX = targetX;
    transitionTargetY = targetY;
    const double dx = targetX - x;
    const double dy = targetY - y;
    const double distance = std::sqrt(dx * dx + dy * dy);
    transitionDuration = std::round(distance / walkspeed);
}

// AS: Enemy16.killMe
void Enemy16::killMe(double dir)
{
    Enemy::killMe(dir);
    if (gameObj->playerObj && gameObj->playerObj->playerData() &&
        gameObj->playerObj->playerData()->customerName == "Prudence") {
        gameObj->obf_94->recordTag("prupickle");
    }
}

} // namespace papaGame::models::enemies
