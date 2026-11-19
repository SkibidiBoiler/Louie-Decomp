#include "game/papaGame/models/bullets/ChiliCan.h"

#include <cmath>

#include "game/as3.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::bullets {

// AS: ChiliCan.ChiliCan
void ChiliCan::init(double x_, double y_, double dirx_, double diry_, const std::string &type_, double id_,
                      double statAttack_, bool fromEnemy_, const std::string &fromWeaponName_)
{
    Bullet::init(x_, y_, dirx_, diry_, type_, id_, statAttack_, fromEnemy_, fromWeaponName_);
    facingDirX = dirx_;
    facingDirY = 0;
    useAngle = diry_;
    fireBullet(x_, y_, dirx_, useAngle);
    isWalking = true;
    jump = true;
    if (speed != 0 && vspeed == 0) {
        throwType = "forward";
    } else if (vspeed < 0 && speed != 0) {
        throwType = "45up";
    } else if (vspeed > 0 && speed != 0) {
        throwType = "45down";
    } else if (vspeed < 0 && speed == 0) {
        throwType = "up";
    } else if (vspeed > 0 && speed == 0) {
        throwType = "down";
    }
    deadZoneRect1.x += x;
    deadZoneRect1.y += y;
    deadZoneRect2.x += x;
    deadZoneRect2.y += y;
    if (facingDirX == -1) {
        deadZoneRect1.x -= deadZoneRect1.width;
        deadZoneRect2.x -= deadZoneRect2.width;
        deadZoneRect2.x -= deadZoneRect1.width * 2;
    }
}

// AS: ChiliCan.defineVars
void ChiliCan::defineVars()
{
    sheetname = "bullet_ChiliCan";
    sheetIsBitmap = false;
    shotname = "ChiliCan";
    width = 6;
    height = 6;
    playerheight = 6;
    spritewidth = 50;
    spriteheight = 50;
    spriteCenterX = 25;
    spriteCenterY = 25;
    spriteTargetX = 25;
    spriteTargetY = 25;
    sheetWidth = 16;
    sheetHeight = 1;
    collRect = Rect{-12, -12, 24, 24};
    heightmultiplier = 1;
    jumpspeed = 0;
    jumpstart = -10;
    gravity = 2;
    normalgravity = 2;
    speed = 10;
    vspeed = 0;
    basespeed = 10;
    flipBlitOffset = 0;
    animationFrame = 0;
    onlyCheckSolidSlopes = true;
    hasDeadZone1 = true;
    hasDeadZone2 = true;
    deadZoneRect1 = Rect{0, -180, 100, 200};
    deadZoneRect2 = Rect{100, -180, 100, 150};
}

// AS: ChiliCan.grabAnimationCycle
const AnimationCycle &ChiliCan::grabAnimationCycle()
{
    return cycleFrames(facingDirX == 1 ? "rightCycleFrames" : "leftCycleFrames");
}

// AS: ChiliCan.fireBullet — QUIRK: `diry` is set to itself (the angle argument is not used here).
void ChiliCan::fireBullet(double px, double py, double pdirx, double /*angle*/)
{
    const display::GameDisplay &display = *gameObj->obf_71;
    x = px;
    y = py;
    dirx = pdirx;
    xtile = std::floor(px / display.tileWidth);
    ytile = std::floor(py / display.tileWidth);
    speed = basespeed;
    jumpspeed = jumpstart;
    isWalking = true;
    jump = true;
}

// AS: ChiliCan.updateObject
void ChiliCan::updateObject()
{
    bool burst = false;
    onlyCheckSolidSlopes = isInDeadZone();
    jumpChar();
    onlyCheckSolidSlopes = isInDeadZone();
    moveChar(dirx, 0, 0);
    if (!jump || onSlope != 0 || !isWalking) {
        burst = true; // it bursts where it lands
    }
    updateSprite();
    bool hitEnemy = false, hitObject = false, hitPlayer = false, hitBullet = false;
    if (fromEnemy) {
        hitPlayer = checkHittingPlayers();
    } else {
        hitEnemy = checkHittingEnemies();
        hitObject = checkHittingObjects(true);
        hitBullet = checkHittingBullets();
    }
    const display::GameDisplay &display = *gameObj->obf_71;
    if (!hitEnemy && !hitObject && !hitPlayer && !hitBullet) {
        if (x + spritewidth + 80 < display.currentXcoord) {
            fadeShot();
        } else if (x > display.currentXcoord + display.screenPxWidth + 80) {
            fadeShot();
        } else if (y > display.levelPxHeight) {
            fadeShot();
        } else if (burst) {
            fadeShot();
        }
    } else {
        fadeShot();
    }
}

// AS: ChiliCan.flyChar — straight flight at speed / vspeed, stopped by walls, clouds and the level's edges.
// (Nothing calls it in PL2.)
bool ChiliCan::flyChar(double, double, double, bool, bool)
{
    const display::GameDisplay &display = *gameObj->obf_71;
    const double tile = display.tileWidth;
    bool stopped = false;
    y += vspeed;
    x += speed;
    getMyCorners(x, y);
    if (speed > 0 && (HitUpRight || HitMidRight || HitDownRight)) {
        stopped = true;
    } else if (speed < 0 && (HitUpLeft || HitMidLeft || HitDownLeft)) {
        stopped = true;
    }
    if (vspeed > 0 && (HitDownLeft || HitDownRight)) {
        stopped = true;
    } else if (vspeed < 0 && (HitUpLeft || HitUpRight)) {
        stopped = true;
    }
    if (checkIfOnCloud() && ytile >= gameObj->playerObj->ytile - 1) {
        stopped = true;
    }
    const double maxX = display.levelTileWidth * tile;
    const double maxY = display.levelTileHeight * tile;
    if (x < 0) {
        x = 0;
        stopped = true;
    }
    if (x > maxX) {
        x = maxX;
        stopped = true;
    }
    if (y < 0) {
        y = 0;
        stopped = true;
    }
    if (y > maxY) {
        y = maxY;
        stopped = true;
    }
    xtile = std::floor(x / tile);
    ytile = std::floor(y / tile);
    return stopped;
}

// AS: ChiliCan.fadeShot — the burst hits everything in its larger rectangle once more and splashes chili about.
void ChiliCan::fadeShot()
{
    isFading = true;
    collRect = blastRect;
    if (fromEnemy) {
        checkHittingPlayers();
    } else {
        checkHittingEnemies();
        checkHittingObjects();
        checkHittingBullets();
    }
    managers::EffectManager &fx = *gameObj->obf_V;
    fx.addEffect(x, y, "ChiliBitEffect", "piece1");
    fx.addEffect(x - 2, y - 2, "ChiliBitEffect", "piece2");
    fx.addEffect(x + 2, y - 1, "ChiliBitEffect", "piece3");
    fx.addEffect(x, y, "BBQBitEffect", "piece1");
    fx.addEffect(x, y - 6, "BBQBitEffect", "piece2");
    fx.addEffect(x, y, "ChiliSplatEffect");
    addSplatter();
    if (std::fmod(id, 2) == 0) {
        gameObj->obf_7C->playSound("chilican1.wav", false, getProximityVolume(), getProximityPan());
    } else {
        gameObj->obf_7C->playSound("chilican2.wav", false, getProximityVolume(), getProximityPan());
    }
    gameObj->obf_4l->removeBullet(id);
}

// AS: ChiliCan.checkHittingEnemies — one enemy per flight (the burst may hit several); the kills are counted
// but not used.
bool ChiliCan::checkHittingEnemies(double, double)
{
    bool hit = false;
    for (Enemy *e : gameObj->obf_E->enemies) {
        if (!e) {
            continue;
        }
        const Rect r{x + collRect.x, y + collRect.y, collRect.width, collRect.height};
        if (e->checkSpriteCollision(r) && !alreadyHitEnemy(e->id)) {
            if (e->id != hitEnemyID && (e->isDead || (!e->isHit && !e->resistBullets))) {
                enemyIDsHit.push_back(e->id);
                const bool landed = e->getHit(statAttack, dirx, true, fromWeaponName);
                addImpactEffect(landed);
                hit = true;
                hitEnemyID = e->id;
                if (!isFading) {
                    break;
                }
            }
        }
    }
    return hit;
}

// AS: ChiliCan.checkHittingObjects
bool ChiliCan::checkHittingObjects(bool blockedToo)
{
    bool hit = false;
    const Rect r{x + collRect.x, y + collRect.y, collRect.width, collRect.height};
    for (GameObject *o : gameObj->obf_7u->objects) {
        if (!o) {
            continue;
        }
        if (o->checkSpriteCollision(r) && o->acceptsDamage("bullet", fromWeaponName)) {
            o->getHit(statAttack);
            hit = true;
            if (!isFading) {
                break;
            }
        } else if (blockedToo && o->checkSpriteCollision(r) && o->blocksBullets) {
            hit = true;
            if (!isFading) {
                break;
            }
        }
    }
    return hit;
}

// AS: ChiliCan.checkHittingPlayers
bool ChiliCan::checkHittingPlayers()
{
    characters::CustomerChar &player = *gameObj->playerObj;
    const Rect r{x + collRect.x, y + collRect.y, collRect.width, collRect.height};
    if (player.checkSpriteCollision(r) && !isStunned && !isDead && player.canGetHit()) {
        player.hurtPlayer(x < player.x ? 1 : -1, statAttack);
        return true;
    }
    return false;
}

// AS: ChiliCan.checkIfOnCloud — clouds do not stop it while it only checks solid slopes.
bool ChiliCan::checkIfOnCloud()
{
    const managers::MapManager &map = *gameObj->obf_4i;
    const double left = map.getTileProperty(leftX, downY, "thrublock");
    const double right = map.getTileProperty(rightX, downY, "thrublock");
    if ((left == 1 || right == 1) && ytile != downY && (!isClimbingLadder || !checkIfLadderBelow())) {
        return !onlyCheckSolidSlopes;
    }
    return false;
}

// AS: ChiliCan.addSplatter — paints a chili stain on the room's tiles where the can burst: the frame fits the slope
// under it, or the floor, wall or ceiling next to it (one of three stains at random).
void ChiliCan::addSplatter()
{
    const display::GameDisplay &display = *gameObj->obf_71;
    const managers::MapManager &map = *gameObj->obf_4i;
    double frame = -1;
    const double slope = map.getTileProperty(xtile, ytile, "slope");
    if (slope > 0) {
        if (slope == 1) {
            frame = 6;
        } else if (slope == 2) {
            frame = 11;
        } else if (slope == 3) {
            frame = 4;
        } else if (slope == 4) {
            frame = 5;
        } else if (slope == 5) {
            frame = 10;
        } else if (slope == 6) {
            frame = 9;
        } else if (slope == 7) {
            frame = 7;
        } else if (slope == 8) {
            frame = 8;
        } else if (slope == 9) {
            frame = 13;
        } else if (slope == 10) {
            frame = 12;
        }
    } else if (!map.getTileProperty(xtile, ytile, "ceilingslope")) {
        if (!map.getTileProperty(xtile, ytile, "collision")) {
            if (map.getTileProperty(xtile, ytile + 1, "naturalcollision")) {
                frame = 2;
            } else if (map.getTileProperty(xtile, ytile + 1, "thrublock")) {
                frame = 2;
            } else if (map.getTileProperty(xtile - 1, ytile, "naturalcollision")) {
                frame = 1;
            } else if (map.getTileProperty(xtile + 1, ytile, "naturalcollision")) {
                frame = 0;
            } else if (map.getTileProperty(xtile, ytile - 1, "naturalcollision")) {
                frame = 3;
            }
        }
    }
    if (frame != -1) {
        if (as3::random() > 0.7) {
            frame += 14;
        } else if (as3::random() > 0.5) {
            frame += 28;
        }
        // Port: the original wraps the point into tileBMP's strips of tilesWidePerRow tiles; the port's tiles are not
        // pre-rendered (see GameDisplay::setupPreBlit), so the stain keeps level coordinates.
        double px = xtile * display.tileWidth - 9;
        double py = ytile * display.tileWidth - 9;
        double offX = x - xtile * display.tileWidth;
        double offY = y - ytile * display.tileWidth;
        if (offX > 16) {
            offX = 16;
        }
        if (offY > 16) {
            offY = 16;
        }
        if (std::fmod(frame, 14) == 2) {
            px += offX;
        } else if (std::fmod(frame, 14) == 0 || std::fmod(frame, 14) == 1) {
            py += offY;
        }
        if (const engine::Sheet *sheet = getSheet()) {
            gameObj->obf_71->stampOnTiles(*sheet, (frame + 2) * spritewidth, 0, spritewidth, spriteheight, px, py);
        }
    }
}

} // namespace papaGame::models::bullets
