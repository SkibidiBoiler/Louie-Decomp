#include "game/papaGame/models/objects/GameObject2.h"

#include <SDL3/SDL.h>

#include <cmath>

#include "engine/audio.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::objects {

// AS: GameObject2.defineVars
void GameObject2::defineVars()
{
    sheetname = "object_pushboulder";
    objectname = "Pushable Boulder";
    sheetIsBitmap = false;
    type = 2;
    spritewidth = 96;
    spriteheight = 96;
    spriteCenterX = 48;
    spriteCenterY = 48;
    spriteTargetX = 48;
    spriteTargetY = 80;
    sheetWidth = 30;
    sheetHeight = 1;
    collRect = {-40, -56, 80, 72};
    activeCycleFrames = cycleType1("active", 1, 0, 0, 0, 0);
    inactiveCycleFrames = cycleType1("inactive", 1, 0, 99, 99, 99);
    activatingCycleFrames = cycleType2("activating", 1, 0, -1, {99});
    deactivatingCycleFrames =
        cycleType2("deactivating", 1, 0, -1, {3, 99, 3, 99, 3, 99, 3, 99, 3, 99, 3, 99, 3, 99, 3, 99, 3, 99, 3, 99});
    isSolidActive = false;
    isSolidInactive = false;
    solidTilesRect = {0, 0, 2, 2};
    solidType = "collision";
    isActivated = true;
    currentHealth = 3;
    maxHealth = 3;
    foreground = false;
    isSteppable = true;
    standingChars.clear();
    standingCharsLastX.clear();
    standingWalkSpeed = 8;
    isPushable = true;
    isGrabbable = false;
    jumpspeed = 0;
    jumpstart = -16;
    jumpgravity = 2;
    normalgravity = 2;
    throwSpeed = 8;
    bounceY = false;
    statAttack = 500;
    canBeShot = false;
    canBeMeleed = false;
    canBeBrokenByObjects = false;
    heightmultiplier = 3;
    widthmultiplier = 1;
    speed = 0;
    walkspeed = 0;
    saveDataAtCheckpoint = true;
}

// AS: GameObject2.saveObjectData — where it was left (also at checkpoints).
void GameObject2::saveObjectData()
{
    const std::vector<as3::Value> row{as3::Value::num(type), as3::Value::num(xtile), as3::Value::num(ytile),
                                      as3::Value::num(currentTriggers)};
    gameObj->obf_AQ->currentScreenData->saveObjectState(id - 1, row, true);
}

// AS: GameObject2.destroy
void GameObject2::destroy()
{
    if (pushSound) {
        pushSound->stop();
        pushSound = nullptr;
    }
    if (!isGrabbed && currentHealth > 0) {
        saveObjectData();
    }
}

// AS: GameObject2.grabAnimationCycle — the boulder turns as it rolls: 3 frames a tick at full speed, 2 from speed
// 3, 1 from speed 2, and 1 every other tick below that, round its 30 frames.
const AnimationCycle &GameObject2::grabAnimationCycle()
{
    if (wasNudged) {
        double frame = movingCycleFrames.frames[0];
        if (rollingSpeed == maxSpeed || rollingSpeed == 0 - maxSpeed) {
            frame += 3 * dirx;
        } else if (std::abs(rollingSpeed) >= 3) {
            frame += 2 * dirx;
        } else if (std::abs(rollingSpeed) >= 2) {
            frame += dirx;
        } else if (rollingSpeed != 0 && std::fmod(gameObj->obf_8o->gameplayTimer, 2) == 0) {
            frame += dirx;
        }
        if (frame >= 30) {
            frame -= 30;
        } else if (frame < 0) {
            frame += 30;
        }
        movingCycleFrames.frames[0] = frame;
    }
    return movingCycleFrames;
}

// AS: GameObject2.checkOnScreen — four columns either side of the screen.
bool GameObject2::checkOnScreen()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 4 && xtile < d.currentXtileRight + 4;
}

// AS: GameObject2.updateObject — it falls and carries its riders; a walking customer who can push rolls it
// (otherwise the player is just held back), it speeds up while pushed, rolls down slopes and slows to a stop, with a
// rumbling sound that follows its speed.
void GameObject2::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    lastx = x;
    lasty = y;
    lastOnSlope = onSlope;
    if (isHit) {
        return;
    }
    fallChar();
    if (jump) {
        jumpChar();
    }
    gameObj->obf_7u->checkSteppingOnObjects(*this, true);
    updateSprite();
    if (jump && jumpspeed >= 0) {
        checkHittingEnemies();
    } else if (wasNudged && speed >= maxSpeed / 2) {
        checkHittingEnemies();
    }
    standOffset = spriteFrameOffsets[static_cast<size_t>(animationFrame)];
    for (int i = static_cast<int>(standingChars.size()) - 1; i >= 0; --i) {
        Character *c = standingChars[static_cast<size_t>(i)];
        c->y = y - spriteTargetY - c->height + standOffset;
        c->ytile = std::floor(c->y / d.tileWidth);
    }

    // The player against it is held at its side; walking into it (a pushing customer) nudges it.
    models::characters::CustomerChar &player = *gameObj->playerObj;
    const Rect box{x + collRect.x, y + collRect.y, collRect.width, collRect.height};
    const double playerX = player.x;
    if (!player.isGrabbing && player.whichMovingTile != this && player.checkSpriteCollision(box)) {
        const bool wasWalking = player.isWalking;
        const double side = player.x < x ? 1 : -1;
        const double target = side == 1 ? x + collRect.x - player.collRect.width - player.collRect.x
                                         : x + collRect.x + collRect.width - player.collRect.x;
        double distance = target - playerX;
        double dir;
        if (distance < 0) {
            dir = -1;
            distance = std::abs(distance);
        } else {
            dir = 1;
        }
        player.moveChar(dir, 0, 0, false, false, true, distance);
        if (player.isRidingObject) {
            player.stopGrabRiding();
        }
        if (!player.isAttacking && wasWalking && player.canPush) {
            wasNudged = true;
            nudgeDirection = side;
            isBeingPushed = true;
            dirx = side;
        } else if (!wasWalking && wasNudged) {
            speed = 0;
            rollingSpeed = 0;
            wasNudged = false;
            dirx = 0;
        } else if (!player.canPush) {
            player.isWalking = false;
        }
    } else {
        isBeingPushed = false;
    }

    if (wasNudged) {
        if (isBeingPushed) {
            if ((rollingSpeed > 0 && nudgeDirection == -1) || (rollingSpeed < 0 && nudgeDirection == 1)) {
                rollingSpeed += 1 * nudgeDirection;
            } else if (rollingSpeed > 0 - maxSpeed && rollingSpeed < maxSpeed) {
                if (std::fmod(speedTimer, speedAcceleration) == 0) {
                    rollingSpeed += 1 * nudgeDirection;
                }
            }
        } else if (player.whichMovingTile == this) {
            // QUIRK: it checks speed (always 0 here), not rollingSpeed.
            if ((speed > maxSpeed || speed < 0 - maxSpeed) && std::fmod(speedTimer, ridingFastDeceleration) == 0) {
                rollingSpeed -= 1 * nudgeDirection;
            } else if (speed > 0 - maxSpeed && speed < maxSpeed) {
                if (std::fmod(speedTimer, ridingDeceleration) == 0) {
                    rollingSpeed -= 1 * nudgeDirection;
                }
            }
        } else if (onSlope != 0) {
            if (onSlope == 1 || onSlope == 3 || onSlope == 4 || onSlope == 7 || onSlope == 8) {
                if (dirx == 1) {
                    rollingSpeed -= 1 * dirx;
                } else {
                    rollingSpeed += 1 * dirx;
                }
            } else if (onSlope == 2 || onSlope == 5 || onSlope == 6 || onSlope == 9 || onSlope == 10) {
                if (dirx == -1) {
                    rollingSpeed -= 1 * dirx;
                } else {
                    rollingSpeed += 1 * dirx;
                }
            }
            if (rollingSpeed > maxSpeed) {
                rollingSpeed = maxSpeed;
            } else if (rollingSpeed < 0 - maxSpeed) {
                rollingSpeed = 0 - maxSpeed;
            }
        } else {
            if (rollingSpeed != 0) {
                if ((speed > maxSpeed || speed < 0 - maxSpeed) && std::fmod(speedTimer, ridingFastDeceleration) == 0) {
                    rollingSpeed -= 1 * dirx;
                } else if (speed >= 0 - maxSpeed && speed <= maxSpeed && std::fmod(speedTimer, speedDeceleration) == 0) {
                    rollingSpeed -= 1 * dirx;
                }
            }
            if (rollingSpeed == 0) {
                // AS: the second test asks getTileProperty for property `false`, which is always undefined.
                if (player.whichMovingTile == this && player.duck &&
                    gameObj->obf_4i->getTileProperty(player.xtile, player.ytile - 1, "collision") == 1) {
                    SDL_Log("Player trapped!  keep going.");
                    rollingSpeed = 2 * dirx;
                } else {
                    rollingSpeed = 0;
                    wasNudged = false;
                    dirx = 0;
                }
            }
        }
        ++speedTimer;
    } else {
        speedTimer = 0;
    }

    if (wasNudged) {
        const double startX = x;
        double step = rollingSpeed;
        const double dir = step < 0 ? -1 : 1;
        dirx = dir;
        step = std::abs(step);
        if (step != 0) {
            moveChar(dir, 0, 0, false, false, true, step);
        }
        for (int i = static_cast<int>(standingChars.size()) - 1; i >= 0; --i) {
            Character *c = standingChars[static_cast<size_t>(i)];
            c->y = y - spriteTargetY - c->height + standOffset;
            c->ytile = std::floor(c->y / d.tileWidth);
        }
        const double endX = x;
        for (int i = static_cast<int>(standingChars.size()) - 1; i >= 0; --i) {
            Character *c = standingChars[static_cast<size_t>(i)];
            double carry = std::abs(endX - startX);
            if (c->isWalking && c->walkingDir == nudgeDirection) {
                carry -= 4;
            }
            if (carry != 0) {
                c->moveChar(nudgeDirection, 0, 0, false, false, true, carry);
            }
            standingCharsLastX[static_cast<size_t>(i)] = c->x;
            const double riderY = y - spriteTargetY - c->height + standOffset;
            if (c->y != riderY) {
                c->y = riderY;
                c->ytile = std::floor(c->y / d.tileWidth);
            }
        }
    }
    if ((wasNudged || isBeingPushed) && (rollingSpeed == 0 || dirx == 0) && onSlope == 0) {
        SDL_Log("STOPIT - %s,%s,%s", as3::String(rollingSpeed).c_str(), as3::String(dirx).c_str(),
                as3::String(onSlope).c_str());
        speed = 0;
        rollingSpeed = 0;
        wasNudged = false;
        isBeingPushed = false;
    }
    if (isBeingPushed && !didFlagFinishTraining) {
        didFlagFinishTraining = gameObj->obf_c("push");
    }
    if ((wasNudged || isBeingPushed) && rollingSpeed != 0 && pushSound == nullptr && !onSlopeWallLeft &&
        !onSlopeWallRight) {
        SDL_Log("Start Sound!");
        pushSound = gameObj->obf_7C->playSound("boulder.wav", true, getProximityVolume(), getProximityPan());
    } else if (pushSound != nullptr && ((!isBeingPushed && rollingSpeed == 0) || onSlopeWallLeft || onSlopeWallRight)) {
        SDL_Log("Stop Sound.");
        pushSound->stop();
        pushSound = nullptr;
    } else if (pushSound != nullptr) {
        const double loudness = std::abs(rollingSpeed) / maxSpeed;
        const double volume = loudness * getProximityVolume();
        pushSound->setTransform(volume, getProximityPan());
        SDL_Log("Volume! -- %s --> %s", as3::String(loudness).c_str(), as3::String(volume).c_str());
    }
}

// AS: GameObject2.checkHittingEnemies — it flattens every live enemy it touches.
bool GameObject2::checkHittingEnemies()
{
    const Rect box{x + collRect.x, y + collRect.y, collRect.width, collRect.height};
    bool hit = false;
    managers::EnemyManager &manager = *gameObj->obf_E;
    const size_t count = manager.enemies.size();
    for (size_t i = 0; i < count && i < manager.enemies.size(); ++i) {
        models::Enemy *e = manager.enemies[i];
        if (e && e->checkSpriteCollision(box)) {
            if (!e->isDead) {
                e->getHit(statAttack, dirx, true, "Boulder");
                hit = true;
            }
        }
    }
    return hit;
}

} // namespace papaGame::models::objects
