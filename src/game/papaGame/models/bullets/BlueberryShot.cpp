#include "game/papaGame/models/bullets/BlueberryShot.h"

#include <cmath>

#include "game/as3.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::bullets {

// AS: BlueberryShot.BlueberryShot — a few blueberry bits fly off Penny's shooter.
void BlueberryShot::init(double x_, double y_, double dirx_, double diry_, const std::string &type_, double id_,
                       double statAttack_, bool fromEnemy_, const std::string &fromWeaponName_)
{
    Bullet::init(x_, y_, dirx_, diry_, type_, id_, statAttack_, fromEnemy_, fromWeaponName_);
    facingDirX = dirx_;
    facingDirY = 0;
    useAngle = diry_;
    fireBullet(x_, y_, dirx_, useAngle);
    gameObj->obf_V->addEffect(x, y, "BlueberryBitEffect", "piece1");
    gameObj->obf_V->addEffect(x - 2, y - 2, "BlueberryBitEffect", "piece2");
    gameObj->obf_V->addEffect(x + 2, y - 1, "BlueberryBitEffect", "piece3");
    isWalking = true;
}

// AS: BlueberryShot.defineVars
void BlueberryShot::defineVars()
{
    sheetname = "bullet_BlueberryShot";
    sheetIsBitmap = false;
    shotname = "BlueberryShot";
    width = 6;
    height = 6;
    playerheight = 6;
    spritewidth = 50;
    spriteheight = 50;
    spriteCenterX = 25;
    spriteCenterY = 25;
    spriteTargetX = 25;
    spriteTargetY = 25;
    sheetWidth = 30;
    sheetHeight = 2;
    collRect = Rect{-8, -8, 16, 24};
    heightmultiplier = 1;
    widthmultiplier = 1;
    jumpspeed = 0;
    jumpstart = 0;
    gravity = 2;
    normalgravity = 2;
    speed = 18;
    vspeed = 0;
    basespeed = 18;
    flipBlitOffset = 0;
    animationFrame = 0;
}

// AS: BlueberryShot.grabAnimationCycle — it turns downwards once its arc starts.
const AnimationCycle &BlueberryShot::grabAnimationCycle()
{
    if (facingDirX == 1) {
        return cycleFrames(arcTimer > arcTimerMax ? "rightDownCycleFrames" : "rightCycleFrames");
    }
    return cycleFrames(arcTimer > arcTimerMax ? "leftDownCycleFrames" : "leftCycleFrames");
}

// AS: BlueberryShot.fireBullet — speed and vspeed from the angle. QUIRK: `diry` is set to itself.
void BlueberryShot::fireBullet(double px, double py, double pdirx, double angle)
{
    const display::GameDisplay &display = *gameObj->obf_71;
    x = px;
    y = py;
    dirx = pdirx;
    xtile = std::floor(px / display.tileWidth);
    ytile = std::floor(py / display.tileWidth);
    const double sx = std::round(basespeed * std::cos(angle) * -1);
    const double sy = std::round(basespeed * std::sin(angle) * -1);
    maxspeedx = sx;
    maxspeedy = sy;
    speed = sx;
    vspeed = sy;
}

// AS: BlueberryShot.updateObject — it flies straight, then bends down a little every other frame. QUIRK: the
// on-screen check is `|| true`.
void BlueberryShot::updateObject()
{
    const display::GameDisplay &display = *gameObj->obf_71;
    ++arcTimer;
    if (arcTimer > arcTimerMax) {
        if (std::fmod(arcTimer, 2) == 0) {
            useAngle += 0.03 * facingDirX;
            speed = std::round(basespeed * std::cos(useAngle) * -1);
            vspeed = std::round(basespeed * std::sin(useAngle) * -1);
        }
    }
    const bool stopped = flyChar(dirx, diry, 0);
    updateSprite();
    bool hitEnemy = false, hitObject = false, hitPlayer = false, hitBullet = false;
    if (fromEnemy) {
        hitPlayer = checkHittingPlayers();
    } else {
        hitEnemy = checkHittingEnemies();
        hitObject = checkHittingObjects(true);
        hitBullet = checkHittingBullets();
    }
    if (!hitEnemy && !hitObject && !hitPlayer && !hitBullet) {
        if (x + spritewidth + 80 < display.currentXcoord) {
            fadeShot();
        } else if (x > display.currentXcoord + display.screenPxWidth + 80) {
            fadeShot();
        } else if (y > display.levelPxHeight) {
            fadeShot();
        } else if (stopped) {
            fadeShot();
        }
    } else {
        fadeShot();
    }
}

// AS: BlueberryShot.flyChar — straight flight at speed / vspeed, stopped by walls, clouds, slopes and the level's
// edges.
bool BlueberryShot::flyChar(double, double, double, bool, bool)
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
    if (checkIfOnCloud()) {
        stopped = true;
    }
    onSlope = gameObj->obf_4i->getTileProperty(xtile, ytile, "slope");
    if (onSlope > 0) {
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

// AS: BlueberryShot.fadeShot — the burst hits everything in its larger rectangle once more.
void BlueberryShot::fadeShot()
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
    fx.addEffect(x, y, "BlueberryBitEffect", "piece1");
    fx.addEffect(x - 2, y - 2, "BlueberryBitEffect", "piece2");
    fx.addEffect(x + 2, y - 1, "BlueberryBitEffect", "piece3");
    fx.addEffect(x, y, "BlueberryBitEffect", "piece1");
    fx.addEffect(x, y - 6, "BlueberryBitEffect", "piece2");
    fx.addEffect(x, y, "BlueberrySplatEffect");
    addSplatter();
    gameObj->obf_7C->playSound("watersplash.wav", false, getProximityVolume(), getProximityPan());
    gameObj->obf_4l->removeBullet(id);
}

// AS: BlueberryShot.checkHittingEnemies — one enemy per flight (the burst may hit several); the kills are counted
// but not used.
bool BlueberryShot::checkHittingEnemies(double, double)
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

// AS: BlueberryShot.checkHittingObjects
bool BlueberryShot::checkHittingObjects(bool blockedToo)
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

// AS: BlueberryShot.checkHittingPlayers
bool BlueberryShot::checkHittingPlayers()
{
    characters::CustomerChar &player = *gameObj->playerObj;
    const Rect r{x + collRect.x, y + collRect.y, collRect.width, collRect.height};
    if (player.checkSpriteCollision(r) && !isStunned && !isDead && player.canGetHit()) {
        player.hurtPlayer(x < player.x ? 1 : -1, statAttack);
        return true;
    }
    return false;
}

// AS: BlueberryShot.addSplatter — paints a blueberry stain on the room's tiles where it burst: the frame fits the slope
// under it, or the floor, wall or ceiling next to it (one of three stains at random). Inside a wall it first steps
// back one tile, against its flight.
void BlueberryShot::addSplatter()
{
    const display::GameDisplay &display = *gameObj->obf_71;
    const managers::MapManager &map = *gameObj->obf_4i;
    double frame = -1;
    double slope = map.getTileProperty(xtile, ytile, "slope");
    if (map.getTileProperty(xtile, ytile, "collision")) {
        if (facingDirX == 1 && map.getTileProperty(xtile - 1, ytile, "collision") == 0) {
            --xtile;
            x -= display.tileWidth;
            slope = map.getTileProperty(xtile, ytile, "slope");
        } else if (facingDirX == -1 && map.getTileProperty(xtile + 1, ytile, "collision") == 0) {
            ++xtile;
            x += display.tileWidth;
            slope = map.getTileProperty(xtile, ytile, "slope");
        }
    }
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
        const double perRow = 30;
        const double row = std::floor(frame / perRow);
        const double col = frame - row * perRow;
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
            gameObj->obf_71->stampOnTiles(*sheet, col * spritewidth, row * spriteheight, spritewidth, spriteheight, px,
                                          py);
        }
    }
}

} // namespace papaGame::models::bullets
