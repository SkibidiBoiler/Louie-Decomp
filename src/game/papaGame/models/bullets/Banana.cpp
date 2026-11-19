#include "game/papaGame/models/bullets/Banana.h"

#include <SDL3/SDL.h>

#include <cmath>

#include "game/as3.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::bullets {

// AS: Banana.Banana — a puff of smoke rings out of Akari's launcher (it follows the player).
void Banana::init(double x_, double y_, double dirx_, double diry_, const std::string &type_, double id_,
                       double statAttack_, bool fromEnemy_, const std::string &fromWeaponName_)
{
    Bullet::init(x_, y_, dirx_, diry_, type_, id_, statAttack_, fromEnemy_, fromWeaponName_);
    facingDirX = dirx_;
    facingDirY = 0;
    useAngle = diry_;
    fireBullet(x_, y_, dirx_, useAngle);
    gameObj->obf_V->addEffect(x, y, "SmokeRingEffect", "", true, x - gameObj->playerObj->x + 20 * dirx_,
                              y - gameObj->playerObj->y);
    isWalking = true;
}

// AS: Banana.defineVars
void Banana::defineVars()
{
    sheetname = "bullet_Banana";
    sheetIsBitmap = false;
    shotname = "Banana";
    width = 6;
    height = 6;
    playerheight = 6;
    spritewidth = 60;
    spriteheight = 46;
    spriteCenterX = 30;
    spriteCenterY = 23;
    spriteTargetX = 30;
    spriteTargetY = 23;
    sheetWidth = 11;
    sheetHeight = 1;
    collRect = Rect{-15, -15, 30, 30};
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
    bounceX = true;
    bounceDecayX = 0.8;
    bounceY = true;
    bounceDecayY = 0.8;
    isFlying = true;
}

// AS: Banana.grabAnimationCycle — it spins the way it faces.
const AnimationCycle &Banana::grabAnimationCycle()
{
    return cycleFrames(facingDirX == 1 ? "rightCycleFrames" : "leftCycleFrames");
}

// AS: Banana.fireBullet — speed and vspeed from the angle. QUIRK: `diry` is set to itself.
void Banana::fireBullet(double px, double py, double pdirx, double angle)
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

// AS: Banana.updateObject — it curves like the roman candle until it hits a wall, a ceiling or the floor; then it
// falls and bounces like a toy, and it goes off after the second bounce. QUIRK: the on-screen check is `|| true`.
void Banana::updateObject()
{
    const display::GameDisplay &display = *gameObj->obf_71;
    bool burst = false;
    const double oldBounces = numBounces;
    if (!didHitSurface) {
        ++arcTimer;
        if (arcTimer > arcTimerMax) {
            if (std::fmod(arcTimer, 2) == 0) {
                useAngle += 0.05 * facingDirX;
                speed = std::round(basespeed * std::cos(useAngle) * -1);
                vspeed = std::round(basespeed * std::sin(useAngle) * -1);
            }
        }
        double wantDirx = 0;
        if (speed < 0) {
            wantDirx = -1;
            dirx = -1;
            moveChar(-1, 0, 0, true, false, true, std::abs(speed));
        } else {
            wantDirx = 1;
            dirx = 1;
            moveChar(1, 0, 0, true, false, true, speed);
        }
        if (dirx != wantDirx) {
            SDL_Log("Bounced off a wall!");
            speed *= -0.8;
            jumpspeed = -4;
            didHitSurface = true;
            if (speed < 0) {
                dirx = -1;
                speed = std::abs(speed);
            } else {
                dirx = 1;
            }
        }
        double wantDiry = 0;
        if (vspeed < 0) {
            diry = -1;
            wantDiry = -1;
            moveChar(0, -1, 0, true, false, true, std::abs(vspeed));
        } else {
            diry = 1;
            wantDiry = 1;
            moveChar(0, 1, 0, true, false, true, vspeed);
        }
        if (diry != wantDiry) {
            SDL_Log("Bounced off a ceiling/floor!");
            vspeed *= -0.8;
            didHitSurface = true;
            jumpspeed = -14;
            if (speed < 0) {
                dirx = -1;
                speed = std::abs(speed);
            } else {
                dirx = 1;
            }
        }
    } else {
        moveChar(dirx, 0, 0, false, false);
        jumpChar();
    }
    if (numBounces != oldBounces) {
        if (isReallyOnScreen()) {
            gameObj->obf_7C->playSound("gummy_bounce" + as3::String(std::ceil(as3::random() * 3)) + ".wav", false,
                                       getProximityVolume(), getProximityPan());
        }
    }
    if (numBounces > 1) {
        burst = true;
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

// AS: Banana.flyChar — not called (left over from the roman candle): straight flight at speed / vspeed, stopped by walls, clouds, slopes and the level's
// edges.
bool Banana::flyChar(double, double, double, bool, bool)
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

// AS: Banana.fadeShot — the burst hits everything in its larger rectangle once more.
void Banana::fadeShot()
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
    fx.addEffect(x, y, "CheeseBitEffect", "piece1");
    fx.addEffect(x - 2, y - 2, "CheeseBitEffect", "piece2");
    fx.addEffect(x + 2, y - 1, "CheeseBitEffect", "piece3");
    fx.addEffect(x, y, "CheeseBitEffect", "piece3");
    gameObj->obf_4l->removeBullet(id);
}

// AS: Banana.checkHittingEnemies — one enemy per flight (the burst may hit several); the kills are counted
// but not used.
bool Banana::checkHittingEnemies(double, double)
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

// AS: Banana.checkHittingObjects
bool Banana::checkHittingObjects(bool blockedToo)
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

// AS: Banana.checkHittingPlayers
bool Banana::checkHittingPlayers()
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
