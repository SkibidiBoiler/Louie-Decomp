#include "game/papaGame/models/enemies/Enemy35.h"

#include <climits>
#include <cmath>

#include <SDL3/SDL.h>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy35.Enemy35 — the Kaiser onions he calls in and both peppers are blitted ahead.
void Enemy35::init(double xtile_, double ytile_, double id_, double whichScreen_, double whichTileset_,
                   double screenObjID_, const std::vector<as3::Value> &extraParams)
{
    Enemy::init(xtile_, ytile_, id_, whichScreen_, whichTileset_, screenObjID_, extraParams);
    gameObj->obf_E->addToPreblit(5, "enemysheet_kaiseronion");
    gameObj->obf_4l->addToPreblit("GiantPepper");
    gameObj->obf_4l->addToPreblit("SargePepper");
}

// AS: Enemy35.defineVars
void Enemy35::defineVars()
{
    type = 35;
    sheetname = "enemysheet_sarge";
    sheetIsBitmap = false;
    height = 15;
    width = 15;
    playerheight = 15;
    spritewidth = 192;
    spriteheight = 160;
    spriteCenterX = 96;
    spriteCenterY = 80;
    spriteTargetX = 96;
    spriteTargetY = 135;
    sheetWidth = 11;
    sheetHeight = 16;
    heightmultiplier = 1;
    widthmultiplier = 1;
    speed = 7;
    walkspeed = 7;
    dirx = -1;
    jumpspeed = 0;
    jumpstart = -12;
    flipBlitOffset = 1280;
    collRect = {-30, -40, 60, 40};
    gravity = 2;
    normalgravity = 2;
    currentHealth = 3;
    maxHealth = 3;
    facingWalking = true;
    points = 10;
    canBeStunned = true;
    stunDuration = 90;
    rememberKilled = true;
    facingDir = -1;
    walkingDir = -1;
    dirx = -1;
    bouncingDoesStun = false;
    enemyCategory = "sarge";
}

// AS: Enemy35.stunEnemy
void Enemy35::stunEnemy()
{
    if (!isStunned) {
        isStunned = true;
    } else {
        cycleFrame = 0;
        animationFrame = cycleFrames("stunCycleFrames").frameAt(0);
    }
    isThrowingLarge = false;
    isThrowingSmall = false;
    isTurning = false;
    if (!didTagStun) {
        gameObj->obf_94->recordTag("enemyStun");
        didTagStun = true;
    }
    stunStartTime = 0;
}

// AS: Enemy35.grabAnimationCycle
const AnimationCycle &Enemy35::grabAnimationCycle()
{
    if (isDead) {
        return cycleFrames("dieCycleFrames");
    }
    if (isHit && !isStartRolling && !isRolling) {
        return cycleFrames(isMinorHurt ? "minorhurtCycleFrames" : "hurtCycleFrames");
    }
    if (isStunned) {
        return cycleFrames("stunCycleFrames");
    }
    if (isRolling) {
        return cycleFrames("rollCycleFrames");
    }
    if (isStartRolling) {
        return cycleFrames("startRollCycleFrames");
    }
    if (jump && jumpspeed < 0) {
        return cycleFrames("jumpCycleFrames");
    }
    if (jump && jumpspeed >= 0) {
        return cycleFrames("fallCycleFrames");
    }
    if (isLanding) {
        return cycleFrames("landCycleFrames");
    }
    if (isThrowingLarge) {
        return cycleFrames("throwLargeCycleFrames");
    }
    if (isThrowingSmall) {
        return cycleFrames("throwSmallCycleFrames");
    }
    if (isTurning) {
        return cycleFrames("turnCycleFrames");
    }
    if (isWalking) {
        return cycleFrames(facingDir == walkingDir ? "walkCycleFrames" : "walkBackwardsCycleFrames");
    }
    return cycleFrames("standCycleFrames");
}

// AS: Enemy35.endAnimationCycle
void Enemy35::endAnimationCycle()
{
    if (cycleName == "turn") {
        isTurning = false;
        if (!isThrowingLarge && !isThrowingSmall) {
            startThrowing();
        }
    } else if (cycleName == "land") {
        isLanding = false;
        finishLanding();
    } else if (cycleName == "stun") {
        isStunned = false;
        startThrowing();
    } else if (cycleName == "startRoll") {
        isStartRolling = false;
        isRolling = true;
        isThrowingLarge = false;
        isThrowingSmall = false;
    } else if (cycleName == "throwLarge") {
        isThrowingLarge = false;
        finishThrowing(true);
    } else if (cycleName == "throwSmall") {
        isThrowingSmall = false;
        finishThrowing();
    } else if (cycleName == "hurt") {
        startRolling();
    } else if (cycleName == "minorhurt") {
        isHit = false;
        isMinorHurt = false;
        decideJump();
    }
}

// AS: Enemy35.sendTriggered — the "Start Fight" trigger drops him into the arena.
void Enemy35::sendTriggered(double /*state*/, bool /*pan*/, bool /*fromGlobal*/)
{
    jump = true;
    jumpspeed = 8;
    isDropping = true;
    gameObj->obf_7C->playTrack("BossTrack", INT_MAX, 0, "crossfade");
}

// AS: Enemy35.startRolling — after a stomp he hops and starts rolling towards the player; the blimps go away.
void Enemy35::startRolling()
{
    isStartRolling = true;
    rollTimer = 0;
    speed = 4;
    jump = true;
    jumpspeed = -8;
    canBeStunned = false;
    isThrowingLarge = false;
    isThrowingSmall = false;
    if (gameObj->playerObj->x < x) {
        facingDir = -1;
        dirx = -1;
        walkingDir = -1;
    } else {
        facingDir = 1;
        dirx = 1;
        walkingDir = 1;
    }
    lastTimeThrownOrJumped = gameObj->obf_8o->gameplayTimer;
    removeBlimps();
}

// AS: Enemy35.stopRolling — a high jump straight up (onto the middle blimp).
void Enemy35::stopRolling()
{
    SDL_Log("Stop Rolling!");
    isRolling = false;
    isStartRolling = false;
    canBeStunned = true;
    jump = true;
    jumpspeed = -28;
    isWalking = false;
    dirx = 0;
    isHit = false;
    lastTimeThrownOrJumped = gameObj->obf_8o->gameplayTimer;
}

// AS: Enemy35.updateObject — nothing happens until he is dropped. Rolling: at 2+ health he rolls from wall to
// wall; at 1 he bounces along. After rollTimerMax frames he stops at x tile 20 and the blimps come back.
// Otherwise he faces the player, throws on frame 6 of a throw, and lands from his jumps.
void Enemy35::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    const characters::CustomerChar &p = *gameObj->playerObj;
    events::GameControls &controls = *gameObj->obf_8o;
    if (controls.keyPressedDown) {
        auto b = [](bool v) { return v ? "true" : "false"; };
        SDL_Log("What's going on: isLanding=%s, isWalking=%s, jump=%s, isTurning=%s, isThrowingLarge=%s, "
                "isThrowingSmall=%s, isRolling=%s, isStartRolling=%s, isHit=%s, isStunned=%s, onMovingTile=%s",
                b(isLanding), b(isWalking), b(jump), b(isTurning), b(isThrowingLarge), b(isThrowingSmall),
                b(isRolling), b(isStartRolling), b(isHit), b(isStunned), b(onMovingTile));
    }
    if (isDead) {
        jumpChar(false);
        moveChar(dirx, 0, 0, false, false, false);
        updateSprite();
        if (ytile > d.currentYtileDown + 1) {
            removeEnemy();
        } else if (isLaunching && (ytile <= d.currentYtileUp - 1 || ytile < 0)) {
            removeEnemy();
        }
        return;
    }
    if (!isDropping && !hasDropped) {
        return;
    }
    const bool wasJumping = jump;
    const bool wasOnMovingTile = onMovingTile;
    const double oldDirX = dirx;
    const double oldFacing = facingDir;
    if (isRolling) {
        isThrowingLarge = false;
        isThrowingSmall = false;
        const double topSpeed = jump ? bounceRollSpeed : rollSpeed;
        if (speed < topSpeed && std::fmod(controls.gameplayTimer, 2) == 0 && rollTimer < rollTimerMax) {
            ++speed;
        }
        if (currentHealth >= 2) {
            moveChar(dirx, 0, 0, true, false);
            facingDir = dirx;
        } else {
            if (dirx == 0 && !jump) {
                if (facingDir == 1) {
                    dirx = -1;
                    facingDir = -1;
                } else {
                    dirx = 1;
                    facingDir = 1;
                }
                if (!isBouncingAndRolling) {
                    isBouncingAndRolling = true;
                    rollTimer = 0;
                }
            }
            if (dirx != 0) {
                facingDir = dirx;
                moveChar(dirx, 0, 0);
                if (!jump && isBouncingAndRolling) {
                    jumpspeed = -30;
                    jump = true;
                    speed = bounceRollSpeed;
                    gameObj->obf_3Y->setCameraJiggle(10, 10);
                    gameObj->obf_7C->playSound("papow.wav");
                }
            }
        }
        if (std::fmod(controls.gameplayTimer, 2) == 0) {
            gameObj->obf_V->addEffect(x, y, "DustEffect", "anim");
        }
        if (dirx != oldDirX) {
            gameObj->obf_3Y->setCameraJiggle(10, 10);
            gameObj->obf_7C->playSound("papow.wav");
        }
        ++rollTimer;
        if (rollTimer > rollTimerMax && xtile == 20) {
            if (currentHealth > 1) {
                addStaticBlimps();
            } else {
                addSecondBlimps();
            }
            stopRolling();
            x = xtile * d.tileWidth + d.tileWidth / 2;
        }
    } else {
        speed = adjustSpeedForSlopes(walkspeed);
        if (!jump && !isLanding && !isStunned && !isHit) {
            if (facingDir == 1 && p.x < x) {
                facingDir = -1;
            } else if (facingDir == -1 && p.x > x) {
                facingDir = 1;
            }
        }
    }
    if ((isThrowingLarge || isThrowingSmall) && cycleFrame == 6) {
        throwPepper();
    }
    fallChar();
    if (jump) {
        if (!isDropping && !isHit && !isRolling) {
            moveChar(dirx, 0, 0, true);
        }
        jumpChar();
    }
    if (isDropping) {
        jumpChar();
    }
    walkingDir = dirx;
    if (facingDir != oldFacing && !isRolling && !isStunned && !isThrowingLarge && !isThrowingSmall &&
        !isStartRolling && !jump) {
        isTurning = true;
    }
    if (onMovingTile) {
        isWalking = false;
    }
    handleSpawningEnemies();
    lookForBlimps();
    gameObj->obf_7u->checkSteppingOnObjects(*this);
    if ((!jump && wasJumping) || (onMovingTile && !wasOnMovingTile)) {
        isWalking = false;
        if (!isMinorHurt) {
            isLanding = true;
            if (!hasDropped) {
                hasDropped = true;
                isDropping = false;
                gameObj->obf_3Y->setCameraJiggle(10, 10);
                gameObj->obf_7C->playSound("papow.wav");
                decideJump(1);
            }
        }
    }
    if (controls.gameplayTimer - lastTimeThrownOrJumped > 45 && !isRolling && !isStartRolling && !isHit &&
        !isStunned && !isLanding && !isThrowingLarge && !isThrowingSmall && !jump) {
        SDL_Log("Haven't moved for a while!  Do something!");
        startThrowing();
    }
    updateSprite();
}

// AS: Enemy35.finishLanding — facing the player he throws (alternating giant and small peppers once hurt);
// otherwise he turns.
void Enemy35::finishLanding()
{
    const characters::CustomerChar &p = *gameObj->playerObj;
    if ((facingDir == 1 && p.x > x) || (facingDir == -1 && p.x < x)) {
        if (!isRolling && !isHit && !isStunned && !isStartRolling && !jump && !lookingForBlimp) {
            if (!isThrowingLarge && !isThrowingSmall) {
                if (currentHealth <= 2) {
                    startThrowing(!lastWasGiant);
                } else {
                    startThrowing();
                }
            }
        }
    } else {
        isTurning = true;
    }
}

// AS: Enemy35.startThrowing — the throw style follows the player's distance: 1 (flat) under 120 px, 2 (high)
// over 360 px, else 0.
void Enemy35::startThrowing(bool giant, double style)
{
    if (isThrowingLarge || isThrowingSmall) {
        return;
    }
    if (giant) {
        isThrowingLarge = true;
        lastWasGiant = true;
    } else {
        isThrowingSmall = true;
        lastWasGiant = false;
    }
    if (style != -1) {
        throwStyle = style;
    } else {
        const double dist = std::abs(x - gameObj->playerObj->x);
        if (dist < 120) {
            throwStyle = 1;
        } else if (dist > 360) {
            throwStyle = 2;
        } else {
            throwStyle = 0;
        }
    }
    walkingDir = facingDir;
    dirx = facingDir;
    ++throwsSinceJump;
    lastTimeThrownOrJumped = gameObj->obf_8o->gameplayTimer;
}

// AS: Enemy35.finishThrowing — QUIRK: every health branch does the same (a jump).
void Enemy35::finishThrowing(bool /*giant*/)
{
    if (currentHealth > 2) {
        decideJump();
    } else if (currentHealth > 1) {
        decideJump();
    } else if (currentHealth > 0) {
        decideJump();
    }
}

// AS: Enemy35.decideJump — dir 1 / -1, or a random side for 0.
void Enemy35::decideJump(double dir)
{
    jumpspeed = -26;
    jump = true;
    if (dir == 1) {
        dirx = 1;
    } else if (dir == -1) {
        dirx = -1;
    } else if (as3::random() > 0.5) {
        dirx = 1;
    } else {
        dirx = -1;
    }
    throwsSinceJump = 0;
    lastTimeThrownOrJumped = gameObj->obf_8o->gameplayTimer;
}

// AS: Enemy35.addStaticBlimps
void Enemy35::addStaticBlimps()
{
    managers::ObjectManager &objects = *gameObj->obf_7u;
    for (const auto &c : staticBlimpCoords) {
        blimps.push_back(objects.addObject(c[1], 0, c[0], whichScreen,
                                           static_cast<double>(objects.objects.size()) + 1,
                                           {as3::Value::num(1), as3::Value::num(c[2])}));
    }
}

// AS: Enemy35.removeBlimps
void Enemy35::removeBlimps()
{
    managers::ObjectManager &objects = *gameObj->obf_7u;
    for (int i = static_cast<int>(blimps.size()) - 1; i >= 0; --i) {
        objects.removeObject(blimps[static_cast<size_t>(i)]);
    }
    blimps.clear();
}

// AS: Enemy35.addSecondBlimps
void Enemy35::addSecondBlimps()
{
    managers::ObjectManager &objects = *gameObj->obf_7u;
    for (const auto &c : secondBlimpCoords) {
        blimps.push_back(objects.addObject(c[1], 0, c[0], whichScreen,
                                           static_cast<double>(objects.objects.size()) + 1,
                                           {as3::Value::num(1), as3::Value::num(c[2])}));
    }
}

// AS: Enemy35.getHit — a bullet or object hit (the third argument) only takes a quarter of a health point and
// makes him flinch; a stomp makes him hurt (then roll).
bool Enemy35::getHit(double damage, double dir, bool unused, const std::string &weapon)
{
    if (unused) {
        damage = 0.25;
    }
    const bool result = Enemy::getHit(damage, dir, unused, weapon);
    if (isHit && unused && as3::round(currentHealth) != currentHealth) {
        isMinorHurt = true;
        jump = true;
        jumpspeed = -8;
    } else {
        isMinorHurt = false;
        jump = true;
        jumpspeed = -16;
    }
    if (isHit) {
        isThrowingLarge = false;
        isThrowingSmall = false;
        isTurning = false;
    }
    if (isDead) {
        gravity = 1;
        normalgravity = 1;
    }
    return result;
}

// AS: Enemy35.killMe — he gets off his blimp, the onions he called in die with him, and the music changes back.
void Enemy35::killMe(double dir)
{
    if (onMovingTile) {
        gameObj->obf_7u->checkSteppingOnObjects(*this);
        if (whichMovingTile) {
            whichMovingTile->stopStandingOnObject(*this);
        }
        whichMovingTile = nullptr;
        onMovingTile = false;
    }
    if (!spawns.empty()) {
        managers::EnemyManager &enemies = *gameObj->obf_E;
        for (double spawnID : spawns) {
            enemies.activateObject(whichScreen, spawnID, false, 1, false);
        }
    }
    if (gameObj->playerObj->playerData()->customerName == "Sarge Fan") {
        gameObj->obf_94->recordTag("sargeFan");
    }
    if (!isDead) {
        gameObj->obf_7C->playTrack("OnionTrack", INT_MAX, 0, "crossfade");
    }
    Enemy::killMe(dir);
}

// AS: Enemy35.throwPepper
void Enemy35::throwPepper()
{
    if (isThrowingLarge) {
        gameObj->obf_4l->addBullet(x + 20 * facingDir, y - 110, facingDir, 0, "GiantPepper", "", true, 1, true);
    } else {
        gameObj->obf_4l->addBullet(x + 20 * facingDir, y - 110, facingDir, throwStyle, "SargePepper", "", true, 1,
                                   true);
    }
}

// AS: Enemy35.lookForBlimps — once hurt, between attacks on the ground he walks until he is under a low blimp,
// then jumps up to it.
void Enemy35::lookForBlimps()
{
    if (currentHealth <= 2 && !onMovingTile && !jump && !isLanding && !isThrowingLarge && !isThrowingSmall &&
        !isRolling && !isStartRolling && !isHit && !isStunned) {
        if (!lookingForBlimp) {
            dirx = facingDir;
            walkingDir = dirx;
        }
        lookingForBlimp = true;
        isWalking = true;
        if (dirx == 0) {
            SDL_Log("(Look for blimp, start moving..)");
            dirx = facingDir;
            walkingDir = dirx;
        }
        moveChar(dirx, 0, 0, true, false);
        const auto &coords = currentHealth > 1 ? staticBlimpCoords : secondBlimpCoords;
        for (const auto &c : coords) {
            if (xtile == c[1] && c[2] == 23) {
                SDL_Log("Found a blimp!");
                lookingForBlimp = false;
                isWalking = false;
                jumpspeed = -28;
                jump = true;
                dirx = 0;
                break;
            }
        }
    }
}

// AS: Enemy35.handleSpawningEnemies — at one hit left, every spawnFrequency frames a Kaiser onion drops in at each
// side of the arena (six in all).
void Enemy35::handleSpawningEnemies()
{
    managers::EnemyManager &enemies = *gameObj->obf_E;
    if (currentHealth <= 1 && !isDead && !isRolling && !isHit && !isStartRolling) {
        ++spawnTimer;
        if (std::fmod(spawnTimer, spawnFrequency) == 0) {
            if (spawnedEnemies < maxToSpawn) {
                const double right = enemies.addEnemy(41, 11, 5, whichScreen, 1, enemies.counter + 1,
                                                      {as3::Value::num(-1), as3::Value::num(0), as3::Value::num(1)});
                ++spawnedEnemies;
                spawns.push_back(right);
            }
            if (spawnedEnemies < maxToSpawn) {
                const double left = enemies.addEnemy(0, 11, 5, whichScreen, 1, enemies.counter + 1,
                                                     {as3::Value::num(1), as3::Value::num(0), as3::Value::num(1)});
                ++spawnedEnemies;
                spawns.push_back(left);
            }
        }
    }
}

} // namespace papaGame::models::enemies
