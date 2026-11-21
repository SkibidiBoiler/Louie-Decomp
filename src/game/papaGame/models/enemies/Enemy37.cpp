#include "game/papaGame/models/enemies/Enemy37.h"

#include <algorithm>
#include <climits>
#include <cmath>

#include "engine/audio.h"
#include "game/as3.h"
#include "game/obf_9G/obf_4R.h"
#include "game/obf_AF/obf_1n.h"

namespace papaGame::models::enemies {

// AS: Enemy37.Enemy37 — he remembers his starting height and preblits the sheets of what he lets out.
void Enemy37::init(double xtile_, double ytile_, double id_, double whichScreen_, double whichTileset_,
                   double screenObjID_, const std::vector<as3::Value> &extraParams)
{
    Enemy::init(xtile_, ytile_, id_, whichScreen_, whichTileset_, screenObjID_, extraParams);
    initialY = y;
    managers::EnemyManager &em = *gameObj->obf_E;
    em.addToPreblit(30, "enemysheet_radishoot");
    em.addToPreblit(34, "enemysheet_burgerzilla");
    em.addToPreblit(38, "enemysheet_RocketBBQ");
    em.addToPreblit(39, "enemysheet_RocketMustard");
}

// AS: Enemy37.defineVars
void Enemy37::defineVars()
{
    type = 37;
    sheetname = "enemysheet_flyingboss";
    sheetIsBitmap = false;
    height = 15;
    width = 15;
    playerheight = 15;
    spritewidth = 170;
    spriteheight = 170;
    spriteCenterX = 85;
    spriteCenterY = 85;
    spriteTargetX = 85;
    spriteTargetY = 130;
    sheetWidth = 16;
    sheetHeight = 18;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 6;
    walkspeed = 6;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -12;
    flipBlitOffset = 1530;
    collRect = {-22, -55, 44, 75};
    gravity = 2;
    normalgravity = 2;
    dieJumpSpeed = -28;
    currentHealth = 3;
    maxHealth = 3;
    facingWalking = true;
    points = 10;
    canBeStunned = true;
    stunDuration = 90;
    facingDir = -1;
    walkingDir = -1;
    dirx = -1;
    rememberKilled = true;
    isFlying = true;
    enemyCategory = "boss";
}

// AS: Enemy37.stunEnemy — only on the ground and not while hurt or taking off.
void Enemy37::stunEnemy()
{
    if (!isFlying && !isReadyJump && !isRising && !isHit && !isDead) {
        if (!isStunned) {
            isStunned = true;
        } else {
            cycleFrame = 0;
            animationFrame = cycleFrames("stunCycleFrames").frameAt(0);
        }
        isUnstunning = false;
        stunStartTime = 0;
        if (!didTagStun) {
            gameObj->obf_94->recordTag("enemyStun");
            didTagStun = true;
        }
    }
}

// AS: Enemy37.sendTriggered — the Start Fight trigger starts him.
void Enemy37::sendTriggered(double /*state*/, bool /*pan*/, bool /*fromGlobal*/)
{
    isTriggered = true;
}

// AS: Enemy37.getHit — flying, only his own mustard rocket counts: it knocks him down (whatever he was doing); on the
// ground only a stunned one takes the hit.
bool Enemy37::getHit(double damage, double dir, bool unused, const std::string &weapon)
{
    if (isFlying && weapon == "RocketMustard") {
        isDropping = true;
        isFlying = false;
        dirx = facingDir;
        rocketCounter = 0;
        isGrabbingRocket = false;
        isLoadingBBQ = false;
        isLoadingMustard = false;
        isQuickLoadingBBQ = false;
        isShooting = false;
        runTimer = 0;
    } else if (!isFlying && isStunned) {
        return Enemy::getHit(damage, dir, unused, weapon);
    }
    return false;
}

// AS: Enemy37.grabAnimationCycle
const AnimationCycle &Enemy37::grabAnimationCycle()
{
    if (isDead) {
        return cycleFrames("dieCycleFrames");
    }
    if (isHit) {
        return cycleFrames("hurtCycleFrames");
    }
    if (isStunned) {
        return cycleFrames("stunCycleFrames");
    }
    if (isUnstunning) {
        return cycleFrames("unstunCycleFrames");
    }
    if (isShooting) {
        return cycleFrames("shootCycleFrames");
    }
    if (isQuickLoadingBBQ) {
        return cycleFrames("quickLoadBBQCycleFrames");
    }
    if (isLoadingBBQ) {
        return cycleFrames("loadBBQCycleFrames");
    }
    if (isLoadingMustard) {
        return cycleFrames("loadMustardCycleFrames");
    }
    if (isGrabbingRocket) {
        return cycleFrames("grabRocketCycleFrames");
    }
    if (isLanding) {
        return cycleFrames("landCycleFrames");
    }
    if (isDropping) {
        return cycleFrames("fallCycleFrames");
    }
    if (isRising) {
        return cycleFrames("jumpCycleFrames");
    }
    if (isReadyJump) {
        return cycleFrames("readyJumpCycleFrames");
    }
    if (isTurning && isFlying) {
        return cycleFrames("turnflyCycleFrames");
    }
    if (isFlying) {
        return cycleFrames("flyCycleFrames");
    }
    if (isTurning && !isFlying) {
        return cycleFrames("turnCycleFrames");
    }
    return cycleFrames("runCycleFrames");
}

// AS: Enemy37.endAnimationCycle — grab, load, shoot in a chain; with 1 hit point a BBQ shot is followed by up to two
// quick reloads; after landing he runs; after a stun or a hurt he gets ready to take off, and rises to 180 px above
// the player.
void Enemy37::endAnimationCycle()
{
    if (cycleName == "turn" || cycleName == "turnfly") {
        isTurning = false;
    } else if (cycleName == "unstun") {
        isUnstunning = false;
        isStunned = false;
        isReadyJump = true;
    } else if (cycleName == "grabRocket") {
        loadRocket();
    } else if (cycleName == "loadBBQ" || cycleName == "loadMustard" || cycleName == "quickLoadBBQ") {
        shootRocket();
    } else if (cycleName == "shoot") {
        isShooting = false;
        if (currentHealth == 1 && useWhichRocket == "BBQ") {
            if (quickLoadCounter < quickLoadCounterMax) {
                isQuickLoadingBBQ = true;
            } else {
                quickLoadCounter = 0;
            }
        }
    } else if (cycleName == "land") {
        isLanding = false;
        isDropping = false;
        isFlying = false;
    } else if (cycleName == "hurt") {
        isHit = false;
        isStunned = false;
        isUnstunning = false;
        isReadyJump = true;
    } else if (cycleName == "readyJump") {
        isReadyJump = false;
        isFlying = true;
        isRising = true;
        riseTargetY = gameObj->playerObj->y - flyTargetYoffset;
    }
}

// AS: Enemy37.startGrabbingRocket — every third rocket is mustard (every sixth with 2 hit points).
void Enemy37::startGrabbingRocket()
{
    isGrabbingRocket = true;
    if (currentHealth == 3) {
        useWhichRocket = std::fmod(rocketCounter + 1, 3) == 0 ? "Mustard" : "BBQ";
    } else if (currentHealth == 2) {
        useWhichRocket = std::fmod(rocketCounter + 1, 6) == 0 ? "Mustard" : "BBQ";
    } else {
        useWhichRocket = std::fmod(rocketCounter + 1, 3) == 0 ? "Mustard" : "BBQ";
    }
    ++rocketCounter;
}

// AS: Enemy37.loadRocket
void Enemy37::loadRocket()
{
    isGrabbingRocket = false;
    if (useWhichRocket == "BBQ") {
        isLoadingBBQ = true;
        if (currentHealth == 1) {
            isQuickLoadingBBQ = true;
        }
    } else {
        gameObj->obf_7C->playSound("dillworm_notice.wav");
        isLoadingMustard = true;
    }
}

// AS: Enemy37.shootRocket — the rocket leaves 50 px in front of him at 10 degrees below the level (the quick
// reloads at -10 and 30 degrees), with a smoke ring.
void Enemy37::shootRocket()
{
    managers::EnemyManager &em = *gameObj->obf_E;
    isGrabbingRocket = false;
    isLoadingBBQ = false;
    isLoadingMustard = false;
    const double rocketType = useWhichRocket == "BBQ" ? 38 : 39;
    const double px = x + 50 * facingDir;
    const double py = y;
    double angle = obf_AF::obf_1n::degreesToRads(facingDir == -1 ? 170 : 10);
    if (isQuickLoadingBBQ) {
        if (facingDir == -1) {
            if (quickLoadCounter == 1) {
                angle = obf_AF::obf_1n::degreesToRads(-170);
            } else if (quickLoadCounter == 2) {
                angle = obf_AF::obf_1n::degreesToRads(150);
            }
        } else if (facingDir == 1) {
            if (quickLoadCounter == 1) {
                angle = obf_AF::obf_1n::degreesToRads(-10);
            } else if (quickLoadCounter == 2) {
                angle = obf_AF::obf_1n::degreesToRads(30);
            }
        }
        ++quickLoadCounter;
        isQuickLoadingBBQ = false;
    }
    em.addEnemy(px, py, rocketType, whichScreen, whichTileset, em.counter + 1,
                {as3::Value::num(angle), as3::Value::num(facingDir), as3::Value::num(id)});
    gameObj->obf_V->addEffect(px, py, "SmokeRingEffect");
    isShooting = true;
    gameObj->obf_7C->playSound("shoot_bazooka.wav");
}

// AS: Enemy37.updateObject — nothing until triggered. Beaten, he falls with starbursts. Flying: the propeller and its
// looping sound, rising to his target or following the player on a wave; idle, he faces the player and grabs a
// rocket on a timer (150 / 70 / 120 frames by hit points). On the ground: he runs, lands, can be stunned, and after
// 120 frames of running gets ready to take off.
void Enemy37::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    managers::EffectManager &fx = *gameObj->obf_V;
    const characters::CustomerChar &p = *gameObj->playerObj;
    if (isTriggered) {
        if (isDead) {
            gravity = 1;
            normalgravity = 1;
            jumpChar(false);
            if (propellerFXID != -1) {
                fx.removeEffect(propellerFXID);
                propellerFXID = -1;
            }
            if (dizzyFXID != -1) {
                fx.removeEffect(dizzyFXID);
                dizzyFXID = -1;
            }
            ++starburstTimer;
            if (starburstTimer >= starburstTimerMax) {
                fx.addEffect(x - 50 + std::round(as3::random() * 100), y - 80 + std::round(as3::random() * 100),
                             "StarburstEffect");
                starburstTimer = 0;
            }
            updateSprite();
            if (ytile > d.currentYtileDown + 3) {
                removeEnemy();
            } else if (isLaunching && (ytile <= d.currentYtileUp - 1 || ytile < 0)) {
                removeEnemy();
            }
        } else if (isFlying) {
            if (currentHealth == 3) {
                grabRocketTimerMax = 150;
            } else if (currentHealth == 2) {
                grabRocketTimerMax = 70;
            } else {
                grabRocketTimerMax = 120;
            }
            if (propellerFXID == -1) {
                propellerFXID = fx.addEffect(x, y, "BossPropellerEffect", as3::String(id), true, 0, -100);
            }
            if (copterSound == nullptr) {
                copterSound = gameObj->obf_7C->playSound("fitzcopter.wav", true, getProximityVolume(), getProximityPan());
            } else {
                copterSound->setTransform(0.5 + getProximityVolume() / 2, getProximityPan());
            }
            if (isRising) {
                const double step = (riseTargetY - initialY) / 8;
                moveChar(0, -1, 0, false, false, true, std::abs(step));
                if (std::abs(step) <= 1) {
                    y = riseTargetY;
                    isRising = false;
                    grabRocketTimer = 50;
                }
                initialY = y;
            } else {
                flyChar();
                ++waveTimer;
                y = initialY + std::sin(waveTimer / wavePeriod) * waveAmplitude;
                ytile = std::floor(y / d.tileWidth);
            }
            if (isIdle()) {
                if (p.x > x && facingDir != 1) {
                    facingDir = 1;
                    isTurning = true;
                } else if (p.x < x && facingDir != -1) {
                    facingDir = -1;
                    isTurning = true;
                }
            }
            if (isIdle()) {
                ++grabRocketTimer;
                if (grabRocketTimer >= grabRocketTimerMax) {
                    startGrabbingRocket();
                    grabRocketTimer = 0;
                }
            }
            handleSpawningEnemies();
            updateSprite();
        } else {
            if (propellerFXID != -1) {
                fx.removeEffect(propellerFXID);
                propellerFXID = -1;
            }
            if (copterSound != nullptr) {
                copterSound->stop();
                copterSound = nullptr;
            }
            speed = adjustSpeedForSlopes(walkspeed);
            const bool wasJumping = jump;
            const double oldDir = dirx;
            if (!isStunned && !isUnstunning && !isHit && !isLanding && !isDropping && !isRising && !isReadyJump) {
                moveChar(dirx, 0, 0, true, false);
                ++runTimer;
            }
            fallChar();
            if (jump) {
                jumpChar();
            }
            facingDir = dirx;
            walkingDir = dirx;
            initialY = y;
            if (dirx != oldDir && !isDropping && !isStunned && !isUnstunning && !isReadyJump && !isHit) {
                isTurning = true;
            }
            if (isStunned) {
                ++stunStartTime;
                if (stunStartTime >= stunDuration) {
                    isStunned = false;
                    isUnstunning = true;
                }
            }
            if (!jump && wasJumping) {
                isLanding = true;
                if (isDropping) {
                    isDropping = false;
                }
            }
            if (runTimer > maxRunTimer && !isStunned && !isUnstunning && !isReadyJump && !isRising && !isDropping &&
                !isHit && !jump) {
                isReadyJump = true;
            }
            updateSprite();
            if (dizzyFXID == -1 && isStunned && cycleFrame == cycleFrames("stunCycleFrames").loopFrame) {
                dizzyFXID = fx.addEffect(x, y - 30, "DizzyEffect", facingDir == 1 ? "faceRight" : "faceLeft");
            } else if (!isStunned && dizzyFXID != -1) {
                fx.removeEffect(dizzyFXID);
                dizzyFXID = -1;
            }
        }
    }
    spriteTargetX = facingDir == 1 ? 67 : 105;
}

// AS: Enemy37.isIdle
bool Enemy37::isIdle() const
{
    return isFlying && !isGrabbingRocket && !isLoadingBBQ && !isLoadingMustard && !isShooting && !isDropping &&
           !isRising;
}

// AS: Enemy37.flyChar — eases towards a point 180 px beside (on the side towards the level's middle) and 180 px
// above the player (80 more while they jump), a 24th of the distance a frame on each axis.
void Enemy37::flyChar()
{
    const characters::CustomerChar &p = *gameObj->playerObj;
    const display::GameDisplay &d = *gameObj->obf_71;
    double targetX = p.x + flyTargetXoffset;
    double targetY = p.y - flyTargetYoffset;
    if (p.jump) {
        targetY -= 80;
    }
    if (p.x > d.levelTileWidth * d.tileWidth / 2) {
        targetX = p.x - flyTargetXoffset;
    }
    const double distx = targetX - x;
    const double disty = targetY - initialY;
    double stepx = distx / 24;
    double stepy = disty / 24;
    double signx = 1;
    double signy = 1;
    if (std::abs(distx) <= 1) {
        stepx = 0;
        x = targetX;
    } else if (stepx > 0 && stepx > distx) {
        stepx = distx;
    } else if (stepx < 0 && stepx < distx) {
        stepx = distx;
    }
    if (std::abs(disty) <= 1) {
        stepy = 0;
        y = targetY;
    } else if (stepy > 0 && stepy > disty) {
        stepy = disty;
    } else if (stepy < 0 && stepy < disty) {
        stepy = disty;
    }
    if (stepx < 0) {
        signx = -1;
        stepx = std::abs(stepx);
    }
    if (stepy < 0) {
        signy = -1;
        stepy = std::abs(stepy);
    }
    dirx = signx;
    diry = signy;
    if (stepy != 0) {
        y = initialY;
        ytile = std::floor(initialY / d.tileWidth);
    }
    if (stepx != 0) {
        moveChar(signx, 0, 0, false, false, true, stepx);
    }
    if (stepy != 0) {
        moveChar(0, signy, 0, false, false, true, stepy);
    }
    initialY = y;
}

// AS: Enemy37.handleSpawningEnemies — every 40 frames while flying: with 3 hit points one Pararadish (Enemy30) above
// the player, with 2 up to three at random columns; with 1, every 30 frames, one Burgerzilla (Enemy34) from a side.
void Enemy37::handleSpawningEnemies()
{
    managers::EnemyManager &em = *gameObj->obf_E;
    if (currentHealth == 3 && !isDead && isFlying && !isRising) {
        ++spawnTimer;
        if (std::fmod(spawnTimer, spawnFrequency) == 0 && spawnedEnemies < 1) {
            spawns.push_back(em.addEnemy(gameObj->playerObj->xtile, 7, 30, whichScreen, 1, em.counter + 1,
                                         {as3::Value::num(screenObjID)}));
            ++spawnedEnemies;
        }
    }
    if (currentHealth == 2 && !isDead && isFlying && !isRising) {
        ++spawnTimer;
        if (std::fmod(spawnTimer, spawnFrequency) == 0 && spawnedEnemies < maxToSpawn) {
            const double col = 4 + std::floor(as3::random() * 28);
            spawns.push_back(em.addEnemy(col, 7, 30, whichScreen, 1, em.counter + 1, {as3::Value::num(screenObjID)}));
            ++spawnedEnemies;
        }
    }
    if (currentHealth == 1 && !isDead && isFlying && !isRising && !isQuickLoadingBBQ && !isShooting &&
        !isGrabbingRocket) {
        ++burgerSpawnTimer;
        if (std::fmod(burgerSpawnTimer, burgerSpawnFrequency) == 0 && spawnedBurgerzillas < maxBurgerzillasToSpawn) {
            double col = 3;
            double facing = 1;
            if (as3::random() > 0.5) {
                col = 32;
                facing = -1;
            }
            burgerSpawns.push_back(em.addEnemy(col, 25, 34, whichScreen, 1, em.counter + 1,
                                               {as3::Value::num(facing), as3::Value::num(0) /* false */,
                                                as3::Value::num(-1), as3::Value::num(1),
                                                as3::Value::num(screenObjID)}));
            ++spawnedBurgerzillas;
        }
    }
}

// AS: Enemy37.unregisterSpawn — a Pararadish or Burgerzilla he let out is gone.
void Enemy37::unregisterSpawn(double spawnType, double spawnID)
{
    auto drop = [spawnID](std::vector<double> &list) {
        auto it = std::find(list.begin(), list.end(), spawnID);
        if (it != list.end()) {
            list.erase(it);
        }
    };
    if (spawnType == 30) {
        --spawnedEnemies;
        drop(spawns);
    } else if (spawnType == 34) {
        --spawnedBurgerzillas;
        drop(burgerSpawns);
    }
}

// AS: Enemy37.killMe — what he let out gets triggered, the Professor Fitz tag, the music stops with the fanfare and
// comes back later (bossDelayMusicTimer).
void Enemy37::killMe(double dir)
{
    managers::EnemyManager &em = *gameObj->obf_E;
    for (double s : spawns) {
        em.activateObject(whichScreen, s, false, 1, false);
    }
    for (double s : burgerSpawns) {
        em.activateObject(whichScreen, s, false, 1, false);
    }
    spawns.clear();
    burgerSpawns.clear();
    maxjumpvelocity = 8;
    characters::CustomerChar &p = *gameObj->playerObj;
    if (p.playerData()->customerName == "Professor Fitz") {
        gameObj->obf_94->recordTag("professor");
    }
    if (!isDead) {
        gameObj->obf_7C->playTrack("", INT_MAX, 0, "outin");
        gameObj->obf_7C->playSound("papalouie2_fanfare.wav");
        if (p.bossDelayMusicTimer == 0) {
            p.bossDelayMusicTimer = 1;
        }
    }
    Enemy::killMe(dir);
}

// AS: Enemy37.destroy
void Enemy37::destroy()
{
    if (copterSound != nullptr) {
        copterSound->stop();
        copterSound = nullptr;
    }
    Enemy::destroy();
}

} // namespace papaGame::models::enemies
