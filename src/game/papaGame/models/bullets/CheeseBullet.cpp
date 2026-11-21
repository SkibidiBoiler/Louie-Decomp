#include "game/papaGame/models/bullets/CheeseBullet.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::bullets {

// AS: CheeseBullet.CheeseBullet — diry carries an angle that fireBullet does not use.
void CheeseBullet::init(double x_, double y_, double dirx_, double diry_, const std::string &type_, double id_,
                       double statAttack_, bool fromEnemy_, const std::string &fromWeaponName_)
{
    Bullet::init(x_, y_, dirx_, diry_, type_, id_, statAttack_, fromEnemy_, fromWeaponName_);
    facingDirX = dirx_;
    facingDirY = 0;
    useAngle = diry_;
    fireBullet(x_, y_, dirx_, useAngle);
    isWalking = true;
    if (facingDirX == 1) {
        animationFrame = 1;
        cycleFrame = 0;
        cycleName = "right";
    }
}

// AS: CheeseBullet.defineVars
void CheeseBullet::defineVars()
{
    sheetname = "bullet_CheeseBullet";
    sheetIsBitmap = false;
    shotname = "CheeseBullet";
    width = 6;
    height = 6;
    playerheight = 6;
    spritewidth = 42;
    spriteheight = 22;
    spriteCenterX = 21;
    spriteCenterY = 11;
    spriteTargetX = 21;
    spriteTargetY = 11;
    sheetWidth = 3;
    sheetHeight = 2;
    collRect = Rect{-10, -6, 20, 12};
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

// AS: CheeseBullet.grabAnimationCycle
const AnimationCycle &CheeseBullet::grabAnimationCycle()
{
    return cycleFrames(facingDirX == 1 ? "rightCycleFrames" : "leftCycleFrames");
}

// AS: CheeseBullet.fireBullet — level flight at basespeed. QUIRK: `diry` is set to itself.
void CheeseBullet::fireBullet(double px, double py, double pdirx, double)
{
    const display::GameDisplay &display = *gameObj->obf_71;
    x = px;
    y = py;
    dirx = pdirx;
    xtile = std::floor(px / display.tileWidth);
    ytile = std::floor(py / display.tileWidth);
    speed = pdirx == 1 ? basespeed : basespeed * -1;
    vspeed = 0;
}

// AS: CheeseBullet.updateObject — it runs everywhere (the original tests `checkOnScreen(this) || true`).
void CheeseBullet::updateObject()
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

// AS: CheeseBullet.flyChar — straight flight, stopped by walls, clouds and the level's edges.
bool CheeseBullet::flyChar(double, double, double, bool, bool)
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

// AS: CheeseBullet.fadeShot — a last check with the blast rectangle, then it splats.
void CheeseBullet::fadeShot()
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
    fx.addEffect(x, y, "CheeseBitEffect", "piece1");
    fx.addEffect(x - 3, y - 1, "CheeseBitEffect", "piece2");
    fx.addEffect(x + 3, y - 1, "CheeseBitEffect", "piece2");
    fx.addEffect(x - 4, y, "CheeseBitEffect", "piece3");
    fx.addEffect(x + 4, y, "CheeseBitEffect", "piece3");
    fx.addEffect(x, y, "CheeseBitEffect", "piece3");
    fx.addEffect(x, y, "CheeseBitEffect", "piece3");
    fx.addEffect(x, y, "CheeseSplatEffect");
    gameObj->obf_4l->removeBullet(id);
}

// AS: CheeseBullet.checkHittingEnemies — one enemy per flight; the kills are counted but not used.
bool CheeseBullet::checkHittingEnemies(double, double)
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

// AS: CheeseBullet.checkHittingObjects
bool CheeseBullet::checkHittingObjects(bool blockedToo)
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

// AS: CheeseBullet.checkHittingPlayers — no damage: the player gets cheesed.
bool CheeseBullet::checkHittingPlayers()
{
    characters::CustomerChar &player = *gameObj->playerObj;
    const Rect r{x + collRect.x, y + collRect.y, collRect.width, collRect.height};
    if (player.checkSpriteCollision(r) && player.canGetHit()) {
        player.getCheesed();
        return true;
    }
    return false;
}

} // namespace papaGame::models::bullets
