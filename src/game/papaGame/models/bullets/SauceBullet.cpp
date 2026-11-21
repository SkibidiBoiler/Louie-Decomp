#include "game/papaGame/models/bullets/SauceBullet.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::bullets {

// AS: SauceBullet.SauceBullet — diry carries an angle that fireBullet does not use.
void SauceBullet::init(double x_, double y_, double dirx_, double diry_, const std::string &type_, double id_,
                       double statAttack_, bool fromEnemy_, const std::string &fromWeaponName_)
{
    Bullet::init(x_, y_, dirx_, diry_, type_, id_, statAttack_, fromEnemy_, fromWeaponName_);
    facingDirX = dirx_;
    facingDirY = 0;
    useAngle = diry_;
    fireBullet(x_, y_, dirx_, useAngle);
}

// AS: SauceBullet.defineVars
void SauceBullet::defineVars()
{
    sheetname = "bullet_SauceBullet";
    sheetIsBitmap = false;
    shotname = "SauceBullet";
    width = 6;
    height = 6;
    playerheight = 6;
    spritewidth = 22;
    spriteheight = 42;
    spriteCenterX = 11;
    spriteCenterY = 21;
    spriteTargetX = 11;
    spriteTargetY = 21;
    sheetWidth = 3;
    sheetHeight = 1;
    collRect = Rect{-5, -10, 10, 20};
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

// AS: SauceBullet.grabAnimationCycle
const AnimationCycle &SauceBullet::grabAnimationCycle()
{
    return cycleFrames("animCycleFrames");
}

// AS: SauceBullet.fireBullet — straight down at basespeed. QUIRK: `diry` is set to itself.
void SauceBullet::fireBullet(double px, double py, double pdirx, double)
{
    const display::GameDisplay &display = *gameObj->obf_71;
    x = px;
    y = py;
    dirx = pdirx;
    xtile = std::floor(px / display.tileWidth);
    ytile = std::floor(py / display.tileWidth);
    speed = 0;
    vspeed = basespeed;
}

// AS: SauceBullet.updateObject — it runs everywhere (the original tests `checkOnScreen(this) || true`).
void SauceBullet::updateObject()
{
    const display::GameDisplay &display = *gameObj->obf_71;
    const bool stopped = flyChar(dirx, diry, 0);
    updateSprite();
    bool hitEnemy = false, hitObject = false, hitPlayer = false;
    if (fromEnemy) {
        hitPlayer = checkHittingPlayers();
    } else {
        hitEnemy = checkHittingEnemies();
        hitObject = checkHittingObjects(true);
    }
    if (!hitEnemy && !hitObject && !hitPlayer) {
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

// AS: SauceBullet.flyChar — straight flight, stopped by walls, clouds and the level's edges.
bool SauceBullet::flyChar(double, double, double, bool, bool)
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

// AS: SauceBullet.fadeShot — a last check with the blast rectangle, then it splats.
void SauceBullet::fadeShot()
{
    isFading = true;
    collRect = blastRect;
    if (fromEnemy) {
        checkHittingPlayers();
    } else {
        checkHittingEnemies();
        checkHittingObjects();
    }
    managers::EffectManager &fx = *gameObj->obf_V;
    fx.addEffect(x, y, "SauceBitEffect", "piece1");
    fx.addEffect(x - 3, y - 1, "SauceBitEffect", "piece2");
    fx.addEffect(x + 3, y - 1, "SauceBitEffect", "piece2");
    fx.addEffect(x - 4, y, "SauceBitEffect", "piece3");
    fx.addEffect(x + 4, y, "SauceBitEffect", "piece3");
    fx.addEffect(x, y, "SauceBitEffect", "piece3");
    fx.addEffect(x, y, "SauceBitEffect", "piece3");
    fx.addEffect(x, y, "SauceSplatEffect");
    gameObj->obf_7C->playSound("watersplash.wav", false, getProximityVolume() / 2, getProximityPan());
    gameObj->obf_4l->removeBullet(id);
}

// AS: SauceBullet.checkHittingEnemies — one enemy per flight; the kills are counted but not used.
bool SauceBullet::checkHittingEnemies(double, double)
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

// AS: SauceBullet.checkHittingObjects
bool SauceBullet::checkHittingObjects(bool blockedToo)
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

// AS: SauceBullet.checkHittingPlayers
bool SauceBullet::checkHittingPlayers()
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
