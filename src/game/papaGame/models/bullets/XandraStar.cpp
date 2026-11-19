#include "game/papaGame/models/bullets/XandraStar.h"

#include <cmath>

#include "game/as3.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::bullets {

// AS: XandraStar.XandraStar — it leaves her hand with a twinkle.
void XandraStar::init(double x_, double y_, double dirx_, double diry_, const std::string &type_, double id_,
                       double statAttack_, bool fromEnemy_, const std::string &fromWeaponName_)
{
    Bullet::init(x_, y_, dirx_, diry_, type_, id_, statAttack_, fromEnemy_, fromWeaponName_);
    facingDirX = dirx_;
    facingDirY = 0;
    useAngle = diry_;
    fireBullet(x_, y_, dirx_, useAngle);
    gameObj->obf_V->addEffect(x, y, "StarEffect", "piece1");
    gameObj->obf_V->addEffect(x, y, "StarEffect", "piece2");
    isWalking = true;
}

// AS: XandraStar.defineVars
void XandraStar::defineVars()
{
    sheetname = "bullet_XandraStar";
    sheetIsBitmap = false;
    shotname = "XandraStar";
    width = 6;
    height = 6;
    playerheight = 6;
    spritewidth = 24;
    spriteheight = 24;
    spriteCenterX = 12;
    spriteCenterY = 12;
    spriteTargetX = 12;
    spriteTargetY = 12;
    sheetWidth = 7;
    sheetHeight = 1;
    collRect = Rect{-10, -10, 20, 20};
    heightmultiplier = 1;
    widthmultiplier = 1;
    jumpspeed = 0;
    jumpstart = 0;
    gravity = 2;
    normalgravity = 2;
    speed = 20;
    vspeed = 0;
    basespeed = 20;
    flipBlitOffset = 0;
    animationFrame = 0;
}

// AS: XandraStar.grabAnimationCycle
const AnimationCycle &XandraStar::grabAnimationCycle()
{
    return cycleFrames("animCycleFrames");
}

// AS: XandraStar.fireBullet — speed and vspeed from the angle. QUIRK: `diry` is set to itself.
void XandraStar::fireBullet(double px, double py, double pdirx, double angle)
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

// AS: XandraStar.updateObject — straight on, leaving a twinkle every other frame.
void XandraStar::updateObject()
{
    const display::GameDisplay &display = *gameObj->obf_71;
    if (display.checkOnScreen(*this)) {
        if (std::fmod(gameObj->obf_8o->gameplayTimer, 2) == 0) {
            gameObj->obf_V->addEffect(x, y, "StarEffect", "static");
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

// AS: XandraStar.flyChar — straight flight at speed / vspeed, stopped by walls, clouds, slopes and the level's
// edges.
bool XandraStar::flyChar(double, double, double, bool, bool)
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

// AS: XandraStar.fadeShot — the burst hits everything in its larger rectangle once more.
void XandraStar::fadeShot()
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
    fx.addEffect(x, y, "StarEffect", "piece1");
    fx.addEffect(x, y, "StarEffect", "piece2");
    fx.addEffect(x, y, "StarEffect", "piece3");
    fx.addEffect(x, y, "StarEffect", "piece1");
    gameObj->obf_4l->removeBullet(id);
}

// AS: XandraStar.checkHittingEnemies — one enemy per flight (the burst may hit several); the kills are counted
// but not used.
bool XandraStar::checkHittingEnemies(double, double)
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

// AS: XandraStar.checkHittingObjects
bool XandraStar::checkHittingObjects(bool blockedToo)
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

// AS: XandraStar.checkHittingPlayers
bool XandraStar::checkHittingPlayers()
{
    characters::CustomerChar &player = *gameObj->playerObj;
    const Rect r{x + collRect.x, y + collRect.y, collRect.width, collRect.height};
    if (player.checkSpriteCollision(r) && !isStunned && !isDead && player.canGetHit()) {
        player.hurtPlayer(x < player.x ? 1 : -1, statAttack);
        return true;
    }
    return false;
}

} // namespace papaGame::models::bullets
