#include "game/papaGame/models/Enemy.h"

#include <algorithm>
#include <cmath>

#include "engine/json.h"
#include "game/obf_9G/obf_4R.h"
#include "game/obf_AF/obf_1n.h"

using obf_AF::obf_1n;

namespace papaGame::models {

Rect Enemy::lookForPlayerRect_, Enemy::keepChasingPlayerRect_;

void Enemy::loadTables(const engine::JsonValue &enemy)
{
    lookForPlayerRect_ = rectField(enemy, "lookForPlayerRect");
    keepChasingPlayerRect_ = rectField(enemy, "keepChasingPlayerRect");
}

Enemy::Enemy(obf_9G::obf_4R *gameObj_) : Character(gameObj_)
{
    lookForPlayerRect = lookForPlayerRect_;
    keepChasingPlayerRect = keepChasingPlayerRect_;
}

// AS: Enemy constructor (Character's constructor calls defineVars first)
void Enemy::init(double xtile_, double ytile_, double id_, double whichScreen_, double whichTileset_,
                 double screenObjID_, const std::vector<as3::Value> &extraParams)
{
    defineVars();
    xtile = xtile_;
    ytile = ytile_;
    id = id_;
    whichScreen = whichScreen_;
    whichTileset = whichTileset_;
    screenObjID = screenObjID_;
    setupParams(extraParams);
    placeEnemy(xtile_, ytile_);
    savePrevious(); // port: nothing to interpolate from yet
}

// AS: Enemy.placeEnemy — enemies standing on a lone tile (or boxed in) do not walk.
void Enemy::placeEnemy(double tileX, double tileY)
{
    const display::GameDisplay &d = *gameObj->obf_71;
    xtile = tileX;
    ytile = tileY;
    x = xtile * d.tileWidth + d.tileWidth / 2;
    y = ytile * d.tileWidth + d.tileWidth / 2;
    const managers::MapManager &m = *gameObj->obf_4i;
    auto solid = [&](double tx, double ty, const char *prop) { return m.getTileProperty(tx, ty, prop) != 0; };
    auto ground = [&](double tx, double ty) { return solid(tx, ty, "collision") || solid(tx, ty, "thrublock"); };
    if (!ground(xtile - 1, ytile + 1) && !ground(xtile + 1, ytile + 1) && ground(xtile, ytile + 1)) {
        onSingleTile = true; // "Enemy is on a single tile, don't walk!"
    } else if (ground(xtile, ytile + 1)) {
        if (solid(xtile + 1, ytile, "collision") && !ground(xtile - 1, ytile + 1)) {
            onSingleTile = true; // "Wall on right, gap on left. Don't walk!"
        } else if (solid(xtile - 1, ytile, "collision") && !ground(xtile + 1, ytile + 1)) {
            onSingleTile = true; // "Wall on left, gap on right. Don't walk!"
        } else if (solid(xtile - 1, ytile, "collision") && solid(xtile + 1, ytile, "collision")) {
            onSingleTile = true; // "Walls on left and right. Don't walk!"
        }
    }
    fallChar();
}

// AS: Enemy.stunEnemy
void Enemy::stunEnemy()
{
    isStunned = true;
    stunStartTime = 0;
}

// AS: Enemy.getHit — true when the hit kills.
bool Enemy::getHit(double damage, double dir, bool /*unused*/, const std::string &weapon)
{
    bool killed = false;
    if (!isDead) {
        currentHealth -= damage;
        isHit = true;
        gameObj->playerObj->addToCombo();
        if (std::find(usedWeapons.begin(), usedWeapons.end(), weapon) == usedWeapons.end()) {
            usedWeapons.push_back(weapon);
        }
        if (currentHealth <= 0 && hitsWhileDead == 0) {
            finalWeapon = weapon;
        }
        if (currentHealth <= 0 &&
            std::find(usedWeaponsJuggling.begin(), usedWeaponsJuggling.end(), weapon) == usedWeaponsJuggling.end()) {
            if (hitsWhileDead > 0) {
                usedWeaponsJuggling.push_back(weapon);
            }
        }
        if (currentHealth <= 0) {
            killMe(dir);
            killed = true;
        }
        // SKELETON: StarburstEffect at (x, y - 24) and the hitenemy_alt.wav sound (effects, sound).
    }
    return killed;
}

// AS: Enemy.killMe — knocked into the air; more hits while dead juggle it higher.
void Enemy::killMe(double dir)
{
    isHit = true;
    isDead = true;
    isFlying = false;
    ++hitsWhileDead;
    speed = dieSpeed;
    dirx = dir;
    currentHealth = 0;
    jumpspeed = dieJumpSpeed / 2;
    if (hitsWhileDead > 1) {
        jumpspeed -= hitsWhileDead * 2;
        if (jumpspeed < dieJumpSpeed) {
            jumpspeed = dieJumpSpeed;
        }
        if (dirx == 0) {
            dirx = as3::random() > 0.5 ? 1 : -1;
        }
        if (hitsWhileDead > 3) {
            speed += 1;
        }
        // AS: the hitsWhileDead >= hitsToLaunch branch is empty.
    }
    jump = true;
    gameObj->obf_51->earnPoints(points);
    if (rememberKilled && hitsWhileDead == 1) {
        gameObj->obf_AQ->currentScreenData->saveEnemyState(id - 1, {as3::Value::num(0)});
    }
}

// AS: Enemy.grabAnimationCycle (returns null)
const AnimationCycle &Enemy::grabAnimationCycle()
{
    return Character::grabAnimationCycle();
}

// AS: Enemy.broadcastTriggered
void Enemy::broadcastTriggered(double /*state*/, bool /*fromGlobal*/)
{
    // SKELETON: tellTriggerArray entries of type 1 call ObjectManager.activateObject (objects).
}

// AS: Enemy.sendTriggered
void Enemy::sendTriggered(double /*state*/, bool /*pan*/, bool /*fromGlobal*/)
{
    killMe(1);
}

// AS: Enemy.checkOnScreen
bool Enemy::checkOnScreen()
{
    return gameObj->obf_71->checkOnScreen(*this);
}

// AS: Enemy.saveEnemyKill
void Enemy::saveEnemyKill()
{
    std::string weapon = "none";
    if (!usedWeapons.empty()) {
        weapon = usedWeapons[0];
    }
    double killID = type;
    if (saveKillAsID != -1) {
        killID = saveKillAsID;
    }
    gameObj->obf_51->saveEnemyKill(killID, weapon, enemyCategory);
}

// AS: Enemy.removeEnemy
void Enemy::removeEnemy()
{
    managers::EnemyManager &manager = *gameObj->obf_E;
    broadcastTriggered(1);
    --manager.jugglingEnemies;
    if (manager.jugglingEnemies < 0) {
        manager.jugglingEnemies = 0;
    }
    manager.removeEnemy(id);
}

// AS: Enemy.jumpChar — falling below the level kills.
void Enemy::jumpChar(bool collide)
{
    Character::jumpChar(collide);
    if (ytile > gameObj->obf_71->levelTileHeight) {
        if (!isDead) {
            killMe(-1);
        }
    }
}

// AS: Enemy.updateObject (the generic walker; the ported types override it)
void Enemy::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    managers::EnemyManager &manager = *gameObj->obf_E;
    jumpChar();
    moveChar(dirx, 0, 0, true, true);
    walkingDir = dirx;
    if (facingWalking) {
        facingDir = walkingDir;
    }
    if (d.checkOnScreen(*this)) {
        updateSprite();
    } else if (xtile < d.currentXtileLeft - 4) {
        manager.removeEnemy(id);
    } else if (y > d.screenPxHeight * 2) {
        manager.removeEnemy(id);
    }
}

// AS: Enemy.destroy
void Enemy::destroy()
{
    if (isDead) {
        saveEnemyKill();
    }
}

// AS: Enemy.lookForPlayer
void Enemy::lookForPlayer()
{
    const auto &p = *gameObj->playerObj;
    if (!noticedPlayer) {
        Rect r{x + lookForPlayerRect.x, y + lookForPlayerRect.y, lookForPlayerRect.width, lookForPlayerRect.height};
        if (facingDir == -1) {
            r.x = x - lookForPlayerRect.x - lookForPlayerRect.width;
        }
        if (p.checkSpriteCollision(r)) {
            noticedPlayer = true;
        }
    } else if (!alwaysNoticePlayer) {
        Rect r{x + keepChasingPlayerRect.x, y + keepChasingPlayerRect.y, keepChasingPlayerRect.width,
               keepChasingPlayerRect.height};
        if (facingDir == -1) {
            r.x = x - keepChasingPlayerRect.x - keepChasingPlayerRect.width;
        }
        if (!p.checkSpriteCollision(r)) {
            noticedPlayer = false;
        }
    }
    if (p.isRidingElevator) {
        noticedPlayer = true;
    }
}

// AS: Enemy.setEventData
void Enemy::setEventData(const std::vector<std::vector<double>> &triggers, double pan, double panTarget)
{
    tellTriggerArray = triggers;
    tellTriggerPan = pan;
    tellTriggerPanTarget = panTarget;
}

// AS: Enemy.setEventTriggers
void Enemy::setEventTriggers(double necessary, const std::vector<std::vector<double>> &from)
{
    necessaryTriggers = std::trunc(necessary);
    fromTriggerArray = from;
}

// AS: Enemy.updateSprite
void Enemy::updateSprite()
{
    if (isMultiSprite) {
        // SKELETON: updateMultiSprite (the enemies with separate legs, head and weapon).
    } else {
        Character::updateSprite();
    }
}

// AS: Enemy.adjustSpeedForSlopes
double Enemy::adjustSpeedForSlopes(double s) const
{
    if (onSlope == 1 || onSlope == 2) {
        return s * std::cos(obf_1n::degreesToRads(45));
    }
    if (onSlope == 3 || onSlope == 4 || onSlope == 5 || onSlope == 6) {
        return s * std::cos(obf_1n::degreesToRads(22));
    }
    if (onSlope == 7 || onSlope == 8) {
        return s * std::cos(obf_1n::degreesToRads(walkingDir == 1 ? 67 : 55));
    }
    if (onSlope == 9 || onSlope == 10) {
        return s * std::cos(obf_1n::degreesToRads(walkingDir == -1 ? 67 : 55));
    }
    return s;
}

} // namespace papaGame::models
