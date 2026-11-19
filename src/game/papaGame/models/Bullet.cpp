#include "game/papaGame/models/Bullet.h"

#include <algorithm>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models {

// AS: Bullet.Bullet — the position is set by the subclasses (fireBullet).
void Bullet::init(double /*x*/, double /*y*/, double dirx_, double diry_, const std::string &type_, double id_,
                  double statAttack_, bool fromEnemy_, const std::string &fromWeaponName_)
{
    defineVars();
    id = id_;
    bulletType = type_;
    dirx = dirx_;
    diry = diry_;
    statAttack = statAttack_;
    fromEnemy = fromEnemy_;
    fromWeaponName = fromWeaponName_;
}

// AS: Bullet.setCritical
void Bullet::setCritical()
{
    isCriticalHit = true;
}

// AS: Bullet.alreadyHitEnemy — only a bullet that passes through enemies remembers them.
bool Bullet::alreadyHitEnemy(double enemyID) const
{
    return canPassThroughEnemies && std::find(enemyIDsHit.begin(), enemyIDsHit.end(), enemyID) != enemyIDsHit.end();
}

// AS: Bullet.checkHittingEnemies
bool Bullet::checkHittingEnemies(double, double)
{
    bool hit = false;
    for (Enemy *e : gameObj->obf_E->enemies) {
        if (!e) {
            continue;
        }
        const Rect r{x + collRect.x, y + collRect.y, collRect.width, collRect.height};
        if (e->checkSpriteCollision(r) && !alreadyHitEnemy(e->id)) {
            if (e->isDead || (!e->isHit && !e->resistBullets)) {
                enemyIDsHit.push_back(e->id);
                const bool landed = e->getHit(statAttack, dirx, true, fromWeaponName);
                addImpactEffect(landed);
                hit = true;
                break;
            }
        }
    }
    if (hit && !canPassThroughEnemies) {
        fadeShot();
    }
    return hit;
}

// AS: Bullet.checkHittingPlayers
bool Bullet::checkHittingPlayers()
{
    characters::CustomerChar &player = *gameObj->playerObj;
    const Rect r{x + collRect.x, y + collRect.y, collRect.width, collRect.height};
    if (player.checkSpriteCollision(r) && !isStunned && !isDead && player.canGetHit()) {
        player.hurtPlayer(x < player.x ? 1 : -1, statAttack);
        addImpactEffect(true);
        fadeShot();
        return true;
    }
    return false;
}

// AS: Bullet.addImpactEffect (it does nothing in the base class)
void Bullet::addImpactEffect(bool) {}

// AS: Bullet.fadeShot
void Bullet::fadeShot()
{
    gameObj->obf_4l->removeBullet(id);
}

// AS: Bullet.checkHittingObjects — `blockedToo`: objects that block bullets stop it as well.
bool Bullet::checkHittingObjects(bool blockedToo)
{
    const Rect r{x + collRect.x, y + collRect.y, collRect.width, collRect.height};
    for (GameObject *o : gameObj->obf_7u->objects) {
        if (!o) {
            continue;
        }
        if (o->checkSpriteCollision(r) && o->acceptsDamage("bullet", fromWeaponName)) {
            o->getHit();
            addImpactEffect();
            fadeShot();
            return true;
        }
        if (blockedToo && o->checkSpriteCollision(r) && o->blocksBullets) {
            addImpactEffect();
            fadeShot();
            return true;
        }
    }
    return false;
}

// AS: Bullet.checkBounce — a thrown bullet bounces twice on the floor, then rolls to a stop.
void Bullet::checkBounce()
{
    const double nextY = jumpspeed + gravity + y;
    const double nextX = x + speed * dirx;
    getMyCorners(nextX, nextY);
    if (jumpspeed >= 0 && isThrown) {
        if (HitDownLeft || HitDownRight || checkIfOnSlope(nextX, nextY)) {
            if (!throwBounce) {
                throwBounce = true;
                bounceY = true;
            } else if (!throwBounce2) {
                throwBounce2 = true;
                bounceY = true;
            } else {
                throwBounce = false;
                throwBounce2 = false;
                bounceY = false;
                dirx = 0;
                isWalking = false;
                isThrown = false;
            }
        }
    }
}

// AS: Bullet.updateObject — straight on until it hits, or leaves the screen sideways.
void Bullet::updateObject()
{
    if (dirx != 0) {
        moveChar(dirx, 0, 0);
    }
    if (diry != 0) {
        moveChar(0, diry, 0);
    }
    checkHittingEnemies();
    checkHittingObjects();
    updateSprite();
    const display::GameDisplay &display = *gameObj->obf_71;
    if (x + spritewidth < display.currentXcoord) {
        fadeShot();
    } else if (x > display.currentXcoord + display.screenPxWidth + 80) {
        fadeShot();
    }
}

// AS: Bullet.grabFlipOffset — the row block for the direction it flies in.
double Bullet::grabFlipOffset() const
{
    double offset = 0;
    if (facingDirX == 1) {
        if (facingDirY == 0) {
            offset = blitOffsetRight;
        } else if (facingDirY == -1) {
            offset = blitOffset45Right;
        } else if (facingDirY == 1) {
            offset = blitOffset45DownRight;
        }
    } else if (facingDirX == -1) {
        if (facingDirY == 0) {
            offset = blitOffsetLeft;
        } else if (facingDirY == -1) {
            offset = blitOffset45Left;
        } else if (facingDirY == 1) {
            offset = blitOffset45DownLeft;
        }
    } else if (facingDirY == 1) {
        offset = blitOffsetDown;
    } else if (facingDirY == -1) {
        offset = blitOffsetUp;
    }
    return offset;
}

// AS: Bullet.isInDeadZone
bool Bullet::isInDeadZone() const
{
    auto inside = [&](const Rect &r) {
        return x >= r.x && x <= r.x + r.width && y >= r.y && y <= r.y + r.height;
    };
    bool in = hasDeadZone1 && inside(deadZoneRect1);
    if (hasDeadZone2 && !in) {
        in = inside(deadZoneRect2);
    }
    return in;
}

const engine::Sheet *Bullet::getSheet() const
{
    const auto &sheets = gameObj->obf_4l->bulletSheets;
    const auto i = static_cast<size_t>(sheetIndex);
    return sheetIndex >= 0 && i < sheets.size() ? sheets[i] : nullptr;
}

} // namespace papaGame::models
