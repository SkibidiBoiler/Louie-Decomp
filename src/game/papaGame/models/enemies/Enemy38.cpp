#include "game/papaGame/models/enemies/Enemy38.h"

#include <algorithm>
#include <climits>
#include <cmath>
#include <numbers>

#include <SDL3/SDL_log.h>

#include "engine/audio.h"
#include "game/as3.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

namespace {
constexpr double PI = std::numbers::pi;
}

// AS: Enemy38.Enemy38 — it starts on the frame of its launch angle.
void Enemy38::init(double xtile_, double ytile_, double id_, double whichScreen_, double whichTileset_,
                   double screenObjID_, const std::vector<as3::Value> &extraParams)
{
    Enemy::init(xtile_, ytile_, id_, whichScreen_, whichTileset_, screenObjID_, extraParams);
    setAnimationAngle();
    animationFrame = grabAnimationCycle().firstFrame;
}

// AS: Enemy38.defineVars
void Enemy38::defineVars()
{
    type = 38;
    sheetname = "enemysheet_RocketBBQ";
    sheetIsBitmap = false;
    width = 12;
    height = 12;
    playerheight = 12;
    spritewidth = 80;
    spriteheight = 80;
    spriteCenterX = 40;
    spriteCenterY = 40;
    spriteTargetX = 40;
    spriteTargetY = 40;
    sheetWidth = 18;
    sheetHeight = 2;
    collRect = {-20, -20, 40, 40};
    heightmultiplier = 1;
    speed = 7;
    widthmultiplier = 1;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -12;
    flipBlitOffset = 0;
    animationFrame = 0;
    gravity = 2;
    normalgravity = 2;
    currentHealth = 1;
    maxHealth = 1;
    facingWalking = true;
    points = 0;
    canBeStunned = true;
    stunDuration = 90;
    foreground = true;
}

// AS: Enemy38.setupParams — [launch angle, facing, boss id].
void Enemy38::setupParams(const std::vector<as3::Value> &params)
{
    if (params.size() > 2) {
        useAngle = params[0].asNumber();
        facingDir = params[1].asNumber();
        dirx = facingDir;
        bossID = params[2].asNumber();
        currentAngle = useAngle;
    }
}

// AS: Enemy38.stunEnemy — a stomp is a hit (not once it heads for the boss).
void Enemy38::stunEnemy()
{
    if (!isSeekingBoss) {
        getHit();
    }
}

// AS: Enemy38.reactToHurtingPlayer — it bursts on the player.
void Enemy38::reactToHurtingPlayer()
{
    fadeShot();
}

// AS: Enemy38.grabAnimationCycle
const AnimationCycle &Enemy38::grabAnimationCycle()
{
    return animCycleFrames;
}

// AS: Enemy38.updateObject — smoke every other frame and its looping engine sound; knocked away it falls spinning
// until it leaves the screen, otherwise it flies and steers (it only hits enemies once it seeks the boss) and bursts
// on a wall.
void Enemy38::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    if (std::fmod(smokeTimer, smokeTimerInterval) == 0) {
        gameObj->obf_V->addEffect(x, y, "DustEffect");
    }
    ++smokeTimer;
    if (rocketSound == nullptr) {
        rocketSound = gameObj->obf_7C->playSound("barrelrocket.wav", true, getProximityVolume(), getProximityPan());
    } else {
        rocketSound->setTransform(getProximityVolume() * 0.5, getProximityPan());
    }
    if (isDeflected) {
        jumpChar(false);
        moveChar(dirx, 0, 0, false, false, false);
        setAnimationAngle();
        updateSprite();
        if (ytile > d.currentYtileDown + 1) {
            removeEnemy();
        } else if (isLaunching && (ytile <= d.currentYtileUp - 1 || ytile < 0)) {
            removeEnemy();
        }
        return;
    }
    const bool stopped = flyChar(dirx, diry, 0);
    setAnimationAngle();
    updateSprite();
    bool hitEnemy = false;
    if (!isSeekingPlayer && isSeekingBoss) {
        hitEnemy = checkHittingEnemies();
    }
    if (hitEnemy || stopped) {
        fadeShot();
    }
}

// AS: Enemy38.fadeShot — the burst (BBQ or mustard splash), a camera jiggle, and it goes.
void Enemy38::fadeShot()
{
    managers::EffectManager &fx = *gameObj->obf_V;
    collRect = blastRect;
    if (!isDeflected && isSeekingBoss) {
        checkHittingEnemies();
    }
    if (rocketType == "RocketBBQ") {
        fx.addEffect(x, y, "ChiliBitEffect", "piece1");
        fx.addEffect(x - 2, y - 2, "ChiliBitEffect", "piece2");
        fx.addEffect(x + 2, y - 1, "ChiliBitEffect", "piece3");
        fx.addEffect(x, y, "BBQBitEffect", "piece1");
        fx.addEffect(x, y - 6, "BBQBitEffect", "piece2");
        fx.addEffect(x, y, "ChiliSplatEffect");
    } else if (rocketType == "RocketMustard") {
        fx.addEffect(x, y, "SauceBitEffect", "piece1");
        fx.addEffect(x - 2, y - 2, "SauceBitEffect", "piece2");
        fx.addEffect(x + 2, y - 1, "SauceBitEffect", "piece3");
        fx.addEffect(x, y, "SauceBitEffect", "piece1");
        fx.addEffect(x, y - 6, "SauceBitEffect", "piece2");
        fx.addEffect(x, y, "SauceSplatEffect");
    }
    gameObj->obf_7C->playSound("watersplash.wav");
    gameObj->obf_3Y->setCameraJiggle(8, 10);
    if (rocketSound != nullptr) {
        rocketSound->stop();
        rocketSound = nullptr;
    }
    removeEnemy();
}

// AS: Enemy38.destroy
void Enemy38::destroy()
{
    if (rocketSound != nullptr) {
        rocketSound->stop();
        rocketSound = nullptr;
    }
    Enemy::destroy();
}

// AS: Enemy38.setAnimationAngle — turns towards the player's middle (or the boss), at most maxAngleDifference a
// frame (knocked away, it just spins 0.3 a frame); the frame is the angle rounded to 10 degrees; the speeds follow.
void Enemy38::setAnimationAngle()
{
    const characters::CustomerChar &player = *gameObj->playerObj;
    double distx = player.x - x;
    double disty = player.y - (player.heightmultiplier - 1) / 2 * player.height - y;
    if (isSeekingBoss) {
        if (const Enemy *boss = gameObj->obf_E->enemyByID(bossID)) {
            distx = boss->x - x;
            disty = boss->y - 20 - y;
        } else {
            distx = 0;
            disty = 0 - y;
        }
    }
    double targetAngle = std::atan2(disty, distx);
    if (isDeflected) {
        targetAngle = currentAngle + 0.3;
    }
    if (currentAngle == INT_MAX) {
        currentAngle = targetAngle;
    }
    double diff = targetAngle - currentAngle;
    if (diff > PI) {
        diff -= PI * 2;
    } else if (diff < PI * -1) {
        diff += PI * 2;
    }
    if (std::abs(diff) > maxAngleDifference) {
        if (diff > 0) {
            targetAngle = currentAngle + maxAngleDifference;
        } else if (diff < 0) {
            targetAngle = currentAngle - maxAngleDifference;
        }
    }
    if (!(targetAngle == PI || targetAngle == PI * -1)) {
        for (int it = 0; it < 10 && targetAngle > PI; ++it) {
            targetAngle -= PI * 2;
        }
        for (int it = 0; it < 10 && targetAngle < PI * -1; ++it) {
            targetAngle += PI * 2;
        }
    }
    const double degrees = radsToDegrees(targetAngle);
    double roundedAngle = std::round(degrees / 10) * 10;
    if (roundedAngle > 180) {
        roundedAngle = -180 + (roundedAngle - 180);
    } else if (roundedAngle < -180) {
        roundedAngle = 180 + (roundedAngle + 180);
    }
    if (roundedAngle == -180) {
        roundedAngle = 180;
    }
    static const double angles[] = {180, -170, -160, -150, -140, -130, -120, -110, -100, -90, -80, -70,
                                    -60, -50,  -40,  -30,  -20,  -10,  0,    10,   20,   30,  40,  50,
                                    60,  70,   80,   90,   100,  110,  120,  130,  140,  150, 160, 170};
    const auto found = std::find(std::begin(angles), std::end(angles), roundedAngle);
    double useFrame = static_cast<double>(found - std::begin(angles));
    if (found == std::end(angles)) {
        SDL_Log("ERROR: Couldn't find %s in Array!!!!", as3::String(roundedAngle).c_str());
        useFrame = 0;
    }
    animCycleFrames.loopFrame = useFrame;
    animCycleFrames.firstFrame = useFrame;
    animCycleFrames.lastFrame = useFrame;
    animationFrame = useFrame;
    currentAngle = targetAngle;
    resetBulletSpeeds();
}

// AS: Enemy38.radsToDegrees
double Enemy38::radsToDegrees(double rads) const
{
    return rads * (180 / PI);
}

// AS: Enemy38.degreesToRads
double Enemy38::degreesToRads(double degrees) const
{
    return degrees * (PI / 180);
}

// AS: Enemy38.matchNegativeSpace (nothing calls it)
double Enemy38::matchNegativeSpace(double angle, double reference) const
{
    if (reference > 0 && angle < 0) {
        angle += PI * 2;
    } else if (reference < 0 && angle > 0) {
        angle -= PI * 2;
    }
    return angle;
}

// AS: Enemy38.placeEnemy — the position comes in pixels; the first speeds point back along the launch angle (the
// first setAnimationAngle sets them again).
void Enemy38::placeEnemy(double px, double py)
{
    const display::GameDisplay &d = *gameObj->obf_71;
    x = px;
    y = py;
    xtile = std::floor(x / d.tileWidth);
    ytile = std::floor(y / d.tileWidth);
    speed = std::round(basespeed * std::cos(useAngle) * -1);
    vspeed = std::round(basespeed * std::sin(useAngle) * -1);
}

// AS: Enemy38.resetBulletSpeeds
void Enemy38::resetBulletSpeeds()
{
    speed = std::round(basespeed * std::cos(currentAngle));
    vspeed = std::round(basespeed * std::sin(currentAngle));
}

// AS: Enemy38.getHit — a BBQ rocket is knocked away spinning (it counts as dead); a mustard rocket turns round and
// homes in on the boss, faster and turning sharper.
bool Enemy38::getHit(double /*damage*/, double dir, bool /*unused*/, const std::string & /*weapon*/)
{
    managers::EffectManager &fx = *gameObj->obf_V;
    canDealDamage = false;
    canBeStunned = false;
    if (rocketType == "RocketBBQ" && !isDeflected) {
        isDeflected = true;
        isSeekingPlayer = false;
        maxAngleDifference = 999999;
        isDead = true;
        speed = 5;
        dirx = dir;
        jump = true;
        jumpspeed = -8;
        currentAngle -= degreesToRads(180);
        if (currentAngle > PI) {
            currentAngle -= PI * 2;
        } else if (currentAngle < PI * -1) {
            currentAngle += PI * 2;
        }
        fx.addEffect(x, y, "StarburstEffect");
        fx.addEffect(x, y, "BBQBitEffect", "piece1");
        fx.addEffect(x, y, "BBQBitEffect", "piece2");
        gameObj->obf_7C->playSound("hitenemy_alt.wav");
        if (rocketSound != nullptr) {
            rocketSound->stop();
            rocketSound = nullptr;
        }
        return true;
    }
    if (rocketType == "RocketMustard" && !isSeekingBoss) {
        isDeflected = false;
        isSeekingPlayer = false;
        isSeekingBoss = true;
        maxAngleDifference = 0.3;
        basespeed = 8;
        currentAngle -= degreesToRads(180);
        if (currentAngle > PI) {
            currentAngle -= PI * 2;
        } else if (currentAngle < PI * -1) {
            currentAngle += PI * 2;
        }
        fx.addEffect(x, y, "StarburstEffect");
        fx.addEffect(x, y, "SauceBitEffect", "piece1");
        fx.addEffect(x, y, "SauceBitEffect", "piece2");
        gameObj->obf_7C->playSound("hitenemy_alt.wav");
        return true;
    }
    return false;
}

// AS: Enemy38.checkHittingEnemies — every enemy it touches (but itself) takes a hit from this rocket type; the
// kills are counted but not used.
bool Enemy38::checkHittingEnemies()
{
    bool hit = false;
    const std::vector<Enemy *> list = gameObj->obf_E->enemies;
    for (Enemy *e : list) {
        if (!e) {
            continue;
        }
        const Rect r{x + collRect.x, y + collRect.y, collRect.width, collRect.height};
        if (e->checkSpriteCollision(r) && e->id != id) {
            if (e->isDead || (!e->isHit && !e->resistBullets)) {
                e->getHit(1, dirx, true, rocketType);
                hit = true;
            }
        }
    }
    return hit;
}

// AS: Enemy38.flyChar — straight flight at speed / vspeed; walls stop it only once it has been clear of them (it
// starts inside the boss's cannon), and never while it seeks the boss.
bool Enemy38::flyChar(double, double, double, bool, bool)
{
    const double tile = gameObj->obf_71->tileWidth;
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
    if (!gotOutOfCollision && !stopped) {
        gotOutOfCollision = true;
    } else if (!gotOutOfCollision) {
        stopped = false;
    }
    if (isSeekingBoss) {
        stopped = false;
    }
    xtile = std::floor(x / tile);
    ytile = std::floor(y / tile);
    return stopped;
}

// AS: Enemy38.jumpChar — a knocked-away rocket dies below the level.
void Enemy38::jumpChar(bool collide)
{
    Enemy::jumpChar(collide);
    if (ytile > gameObj->obf_71->levelTileHeight) {
        if (!isDead && isDeflected) {
            killMe(-1);
        }
    }
}

// AS: Enemy38.killMe — the original also traces "HHEEEEEY, ROCKET KILLME".
void Enemy38::killMe(double dir)
{
    SDL_Log("HHEEEEEY, ROCKET KILLME");
    Enemy::killMe(dir);
}

} // namespace papaGame::models::enemies
