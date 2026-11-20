#include "game/papaGame/models/enemies/Enemy26.h"

#include <cmath>
#include <numbers>

#include <SDL3/SDL.h>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy26.Enemy26
void Enemy26::init(double xtile_, double ytile_, double id_, double whichScreen_, double whichTileset_,
                   double screenObjID_, const std::vector<as3::Value> &extraParams)
{
    Enemy::init(xtile_, ytile_, id_, whichScreen_, whichTileset_, screenObjID_, extraParams);
    startupEnemy();
}

// AS: Enemy26.defineVars
void Enemy26::defineVars()
{
    type = 26;
    sheetname = "enemysheet_cheddarmack";
    sheetIsBitmap = false;
    height = 15;
    width = 15;
    playerheight = 15;
    spritewidth = 90;
    spriteheight = 60;
    spriteCenterX = 45;
    spriteCenterY = 30;
    spriteTargetX = 45;
    spriteTargetY = 10;
    sheetWidth = 18;
    sheetHeight = 2;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 2;
    walkspeed = 2;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -12;
    flipBlitOffset = 0;
    collRect = {-15, -20, 26, 26};
    gravity = 2;
    normalgravity = 2;
    currentHealth = 1;
    maxHealth = 1;
    facingWalking = true;
    points = 10;
    canBeStunned = true;
    stunDuration = 90;
    enemyCategory = "cheese";
}

// AS: Enemy26.getHit — a hit pushes the swing that way (a direction of 0 counts as 1).
bool Enemy26::getHit(double damage, double dir, bool unused, const std::string &weapon)
{
    if (dir == 0) {
        dir = 1;
    }
    if (!isDead) {
        setupSwing(dir);
    }
    return Enemy::getHit(damage, dir, unused, weapon);
}

// AS: Enemy26.stunEnemy — a stomp is a hit, away from the player.
void Enemy26::stunEnemy()
{
    getHit(1, gameObj->playerObj->x > x ? -1 : 1, false, "stomp");
}

// AS: Enemy26.grabAnimationCycle
const AnimationCycle &Enemy26::grabAnimationCycle()
{
    if (isDead) {
        return cycleFrames("dieCycleFrames");
    }
    return swingCycleFrames;
}

// AS: Enemy26.updateObject — beaten, it lets go of the rope and falls; otherwise it keeps swinging (a stopped swing
// starts again to a random side), creaking as it passes the bottom. Its hit box follows the frame.
void Enemy26::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    if (isDead) {
        if (isSwinging) {
            SDL_Log("Detach and drop!");
            isSwinging = false;
            isConnected = false;
            jumpspeed = -4;
            jump = true;
        }
        jumpChar(false);
        moveChar(dirx, 0, 0, false, false, false);
        updateSprite();
        collRect.x = collX[static_cast<size_t>(animationFrame)];
        collRect.y = collY[static_cast<size_t>(animationFrame)];
        if (ytile > d.currentYtileDown + 1) {
            removeEnemy();
        } else if (isLaunching && (ytile <= d.currentYtileUp - 1 || ytile < 0)) {
            removeEnemy();
        }
        return;
    }
    if (!isSwinging) { // (the original tests `(_loc7_ = true) && !isSwinging`)
        const double side = as3::random() > 0.5 ? -1 : 1;
        setupSwing(side);
        dirx = side;
    }
    if (isSwinging) {
        swingChar();
    }
    const double degrees = as3::round(convertToDegrees(currentAngle));
    if (degrees > 85 && degrees < 95 && gameObj->obf_8o->gameplayTimer - lastPlayedSound > 10) {
        if (isReallyOnScreen()) {
            gameObj->obf_7C->playSound("mackswing.wav", false, getProximityVolume(), getProximityPan());
        }
        lastPlayedSound = gameObj->obf_8o->gameplayTimer;
    }
    updateAnimationFrame();
    updateSprite();
    collRect.x = collX[static_cast<size_t>(animationFrame)];
    collRect.y = collY[static_cast<size_t>(animationFrame)];
}

// AS: Enemy26.startupEnemy — centred on its tile; the rope hangs from the first solid tile above (or the top of the
// level).
void Enemy26::startupEnemy()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    const managers::MapManager &map = *gameObj->obf_4i;
    x = xtile * d.tileWidth + d.tileWidth / 2;
    y = ytile * d.tileWidth + d.tileWidth / 2;
    for (int row = static_cast<int>(ytile); row >= 0; --row) {
        if (map.getTileProperty(xtile, row, "collision") == 1) {
            hasAnchorTile = true;
            anchorTileX = xtile;
            anchorTileY = row;
            break;
        }
    }
    if (hasAnchorTile) {
        anchorX = anchorTileX * d.tileWidth + d.tileWidth / 2;
        anchorY = (anchorTileY + 1) * d.tileWidth;
    } else {
        anchorX = x;
        anchorY = 0;
    }
}

// AS: Enemy26.convertToRadians
double Enemy26::convertToRadians(double degrees) const
{
    return degrees * (std::numbers::pi / 180);
}

// AS: Enemy26.convertToDegrees
double Enemy26::convertToDegrees(double radians) const
{
    return radians * (180 / std::numbers::pi);
}

// AS: Enemy26.setupSwing — starts a pendulum swing from 130° (dir 1) or 50° (dir -1), timed from where it hangs
// now; fromCurrentAngle starts it from its present angle instead. (Its debug trace() calls are kept as logs; it
// also reads gameObj.displayObj and never uses it.)
void Enemy26::setupSwing(double dir, bool fromCurrentAngle)
{
    SDL_Log("START SWINGING!");
    swingSpeed = 1;
    isSwinging = true;
    const double dx = x - anchorX;
    const double dy = y - anchorY;
    lineLength = std::sqrt(dx * dx + dy * dy);
    const double period = 2 * std::numbers::pi * std::sqrt(lineLength / swingSpeed);
    timeElapsed = as3::round(period / 4);
    if (fromCurrentAngle) {
        initialAngle = std::atan2(dy, dx);
        currentAngle = initialAngle;
        workingAngle = initialAngle - convertToRadians(90);
        if (workingAngle < convertToRadians(-180)) {
            workingAngle += convertToRadians(360);
        }
        targetAngle = convertToRadians(180) - initialAngle;
        pumpAdjustor = gravityDecay + 0.02;
        timeElapsed = 1;
    } else {
        initialAngle = std::atan2(dy, dx);
        const double highDegrees = 130;
        const double lowDegrees = 50;
        const double high = convertToRadians(highDegrees);
        const double low = convertToRadians(lowDegrees);
        if (as3::round(convertToDegrees(initialAngle)) == 90) {
            SDL_Log("AT 90.  START IT AT 1/4 PERIOD!");
            timeElapsed = as3::round(period / 4);
            pumpAdjustor = gravityDecay + 0.03;
        } else {
            const double range = highDegrees - lowDegrees;
            const double fromLow = as3::round(convertToDegrees(initialAngle)) - lowDegrees;
            const double percent = std::floor(fromLow / range * 50);
            if (dir == -1) {
                timeElapsed = as3::round(period * (percent / 100));
            } else if (dir == 1) {
                timeElapsed = as3::round(period * ((50 - percent) / 100));
            }
            SDL_Log("MID-SWING -- %s - %s%% ACROSS. - TIME: %s / %s",
                    as3::String(as3::round(convertToDegrees(initialAngle))).c_str(), as3::String(percent).c_str(),
                    as3::String(timeElapsed).c_str(), as3::String(period).c_str());
            pumpAdjustor = gravityDecay + 0.03;
        }
        currentAngle = dir == 1 ? high : low;
        workingAngle = currentAngle - convertToRadians(90);
        if (workingAngle < convertToRadians(-180)) {
            workingAngle += convertToRadians(360);
        }
        targetAngle = convertToRadians(180) - currentAngle;
    }
    changeSlackDir = 0;
    SDL_Log("FINAL: INITIAL ANGLE: %s, WORKING ANGLE: %s, TARGET ANGLE: %s",
            as3::String(convertToDegrees(initialAngle)).c_str(), as3::String(convertToDegrees(workingAngle)).c_str(),
            as3::String(convertToDegrees(targetAngle)).c_str());
}

// AS: Enemy26.swingChar — one step of a damped pendulum: the amplitude (workingAngle, at most maxAngle) shrinks by
// pumpAdjustor each frame; it stops hanging straight down, and restarts its timing when it swings over the anchor.
void Enemy26::swingChar()
{
    const double time = timeElapsed;
    const double angle = currentAngle;
    const double velocity = currentVelocity;
    ++pumpDropTimer;
    if (collidedWhileSwinging) {
        return;
    }
    workingAngle = convertToRadians(convertToDegrees(workingAngle) * pumpAdjustor);
    if (convertToDegrees(workingAngle) > maxAngle) {
        workingAngle = convertToRadians(maxAngle);
    } else if (convertToDegrees(workingAngle) < 0 - maxAngle) {
        workingAngle = convertToRadians(0 - maxAngle);
    }
    double next = workingAngle * std::cos(std::sqrt(swingSpeed / lineLength) * time);
    next = next + convertToRadians(90);
    if (convertToDegrees(next) > 180) {
        next = convertToRadians(-180 + (convertToDegrees(next) - 180));
    }
    const double step = next - angle;
    auto restartFrom = [&](double a) {
        timeElapsed = 1;
        initialAngle = a;
        currentAngle = a;
        workingAngle = a - convertToRadians(90);
        if (workingAngle < convertToRadians(-180)) {
            workingAngle += convertToRadians(360);
        }
        targetAngle = convertToRadians(180) - initialAngle;
        pumpAdjustor = gravityDecay;
        pumpDropTimer = 0;
    };
    if (std::abs(convertToDegrees(step)) < 0.2 && as3::round(convertToDegrees(next)) == 90) {
        currentAngle = convertToRadians(90);
        workingAngle = 0;
        pumpAdjustor = gravityDecay;
        pumpDropTimer = 0;
        isSwinging = false;
        SDL_Log("STOP SWINGING.");
    } else if (convertToDegrees(next) < 0 && convertToDegrees(next) > -90 && convertToDegrees(angle) < -90 &&
               velocity > 0) {
        restartFrom(next);
    } else if (convertToDegrees(next) < -90 && convertToDegrees(next) > -180 && convertToDegrees(angle) > -90 &&
               velocity < 0) {
        restartFrom(next);
    } else {
        currentAngle += step;
    }
    timeElapsed += 1;
    currentVelocity = convertToDegrees(step);
    const double offsetX = as3::round(std::cos(currentAngle) * lineLength);
    const double offsetY = as3::round(std::sin(currentAngle) * lineLength);
    x = anchorX + offsetX - ropeOffsetX;
    y = anchorY + offsetY - ropeOffsetY;
    xtile = std::floor(x / gameObj->obf_71->tileWidth);
    ytile = std::floor(y / gameObj->obf_71->tileWidth);
}

// AS: Enemy26.checkBeforeBlitting — the rope is drawn under it while it hangs on.
void Enemy26::checkBeforeBlitting()
{
    if (isConnected) {
        redrawLine();
    }
}

// AS: Enemy26.checkOnScreen — it, or its rope's anchor tile, on screen. (With no anchor tile the original's
// anchorTile.x throws; the port counts only the enemy then.)
bool Enemy26::checkOnScreen()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    if (d.checkOnScreen(*this)) {
        return true;
    }
    if (!hasAnchorTile) {
        return false;
    }
    return anchorTileX >= d.currentXtileLeft - 1 && anchorTileX <= d.currentXtileRight + 1 &&
           anchorTileY >= d.currentYtileUp - 1 && anchorTileY <= d.currentYtileDown + 1;
}

// AS: Enemy26.redrawLine — a brown rope (4 px, 0xB17400) with a yellow core (2 px, 0xF3C000) from it to the anchor.
// Port: the cheese end follows the enemy's drawn (interpolated) position.
void Enemy26::redrawLine()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    const double fromX = d.drawnX(*this) - d.currentXcoord;
    const double fromY = d.drawnY(*this) - d.currentYcoord;
    const double toX = anchorX - d.currentXcoord;
    const double toY = anchorY - d.currentYcoord;
    d.drawScreenLine(fromX, fromY, toX, toY, 4, 11629824);
    d.drawScreenLine(fromX, fromY, toX, toY, 2, 15974912);
}

// AS: Enemy26.updateAnimationFrame — the frame shows the rope's angle in 4° steps: 16 hanging straight down
// (88°–92°), 0 at 152° and over, 31 under 32°. (The original spells out every 4° band in an if chain; a NaN angle
// keeps 16.)
void Enemy26::updateAnimationFrame()
{
    const double degrees = convertToDegrees(currentAngle);
    double frame = 16;
    if (degrees >= 88 && degrees < 92) {
        frame = 16;
    } else if (degrees >= 92 && degrees < 152) {
        frame = 15 - std::floor((degrees - 92) / 4); // 92–96: 15 ... 148–152: 1
    } else if (degrees >= 152) {
        frame = 0;
    } else if (degrees >= 32 && degrees < 88) {
        frame = 16 + std::ceil((88 - degrees) / 4); // 84–88: 17 ... 32–36: 30
    } else if (degrees < 32) {
        frame = 31;
    }
    swingCycleFrames.frames[0] = frame;
}

// AS: Enemy26.killMe — Papa Louie beating one earns a tag.
void Enemy26::killMe(double dir)
{
    Enemy::killMe(dir);
    if (gameObj->playerObj && gameObj->playerObj->playerData()->customerName == "Papa Louie") {
        gameObj->obf_94->recordTag("papamack");
    }
}

} // namespace papaGame::models::enemies
