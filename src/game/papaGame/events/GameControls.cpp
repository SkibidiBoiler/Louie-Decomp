#include "game/papaGame/events/GameControls.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::events {

using models::characters::CustomerChar;
using papaGame::data::CustomerData;

// AS: GameControls.resetKeys
void GameControls::resetKeys()
{
    keyPressedAction = keyPressedJump = keyPressedLeft = keyPressedRight = keyPressedUp = keyPressedDown = false;
}

// AS: GameControls.detectKeys (player part)
void GameControls::detectKeys()
{
    CustomerChar &p = *gameObj->playerObj;
    const bool wasJumping = p.jump;
    if (isPaused || stopCycle) {
        return;
    }
    if (!p.isFinishingLevel) {
        ++gameplayTimer;
        ++currentLevelTimer;
    }
    // SKELETON: UserData timeBonus / levelTimePlayed.
    if (p.isShoved && ((p.walkingDir == 1 && keyPressedRight) || (p.walkingDir == -1 && keyPressedLeft))) {
        p.isShoved = false;
        p.walkingDir = p.facingDir;
    }
    if (!stopControls && !stopHurtControls && !stopActionControls) {
        keyCheckPlayer(p);
    }
    if (p.jump) {
        p.duck = false;
        p.isDucking = false;
        p.isCrawling = false;
        if (p.jumpspeed < 0) {
            p.isClimbingLadder = false;
        }
        if (p.isDead && p.isRidingElevator) {
            p.jumpChar(false);
        } else {
            p.jumpChar();
            // SKELETON: ground pounding runs updateObject and jumpChar once more.
        }
    }
    if (wasJumping && !p.jump && !p.isWalking && p.skidSpeed != 0 && !p.isHit) {
        p.skidSpeed = 0;
    }
    if (!p.isWalking && p.jump && p.skidSpeed != 0 && !p.isHit) {
        p.skidSpeed = 0;
    }
    if (wasJumping && !p.jump && !p.isHit) {
        // Landing: face the way the player looks, and drop any backwards air speed.
        p.walkingDir = p.facingDir;
        p.isTurning = false;
        if (p.speed < 0) {
            p.speed *= -1;
        }
        if (p.skidSpeed > p.speed) {
            p.skidSpeed = p.speed;
        }
    }
    if (p.triggerjumpx) {
        p.moveChar(p.walkingDir, 0, 0);
    }
    if (p.isDead && p.jump) {
        // A dead player flies off in the direction of the hit (SKELETON: falling off a train at speed 30).
        p.setSpeed(p.walkspeed);
        if (p.isRidingElevator) {
            p.moveChar(p.walkingDir, 0, 0, false, false, false);
        } else {
            p.moveChar(p.walkingDir, 0, 0);
        }
    }
    if (!p.isDead && (p.isHit || p.isShoved)) {
        if (p.speed > 0) {
            p.moveChar(p.walkingDir, 0, 0);
        }
    }
    p.updateObject();
    gameObj->obf_3Z->processItems();
    gameObj->obf_4l->processBullets();
    gameObj->obf_E->processEnemies();
    gameObj->obf_V->processEffects();
    gameObj->obf_7u->processObjects();
    // SKELETON: cameraAnchor (setCameraAnchor) is not ported; the anchor is always the player (cameraAnchor 0).
    gameObj->obf_3Y->adjustCamera(p.x, p.y, true);
    gameObj->obf_6Z->updateTimer();
    // redrawScreen comes next in the original; the port redraws in its render pass.
}

// AS: GameControls.jumpPlayer
void GameControls::jumpPlayer(CustomerChar &p)
{
    const bool jumpKey = keyPressedJump;
    const bool downKey = keyPressedDown;
    if (downKey && !p.jump && p.isGrabbing) {
        // Down while hanging: drop.
        p.jump = true;
        p.jumpspeed = 2;
        p.jumpedFromSlope = p.onSlope;
        p.jumpedFromX = p.x;
        p.jumpedFromY = p.y;
        const double tw = gameObj->obf_71->tileWidth;
        const double belowY = std::floor((p.y + p.height - 1 + 22) / tw);
        const double leftX = std::floor((p.x - p.width) / tw);
        const double rightX = std::floor((p.x + p.width * p.widthmultiplier - 1) / tw);
        const bool leftHit = gameObj->obf_4i->getTileProperty(leftX, belowY, "collision") != 0;
        const bool rightHit = gameObj->obf_4i->getTileProperty(rightX, belowY, "collision") != 0;
        if (!leftHit && !rightHit) {
            p.y += 22;
        }
        p.ytile = std::floor(p.y / tw);
        p.gravity = p.normalgravity;
        p.duck = false;
        p.isDucking = false;
        p.isGrabbing = false;
        p.isStartingGrabbing = false;
        p.isWalking = false;
    }
    const bool free = !p.isCheesed && !p.isInQuicksand;
    const bool canDouble = p.skillType == CustomerData::SKILL_DOUBLEJUMP && !p.isDoubleJumping && free;
    const bool canGlide = p.skillType == CustomerData::SKILL_GLIDE && !p.isGliding && free;
    if (jumpKey && (!p.jump || p.canFudgeJump() || canDouble || canGlide) && !p.isGrabbingObject &&
        (p.canPressJump || (p.canPressDoubleJump && (canDouble || canGlide))) &&
        (!p.duck || (p.skillType != CustomerData::SKILL_CRAWL || !p.isCrawling)) && !p.isRidingElevator &&
        !p.isSliding && !p.isClimbingLadder && !p.checkIfOnLadder(-1) &&
        (!p.isRidingObject || (p.whichObjectRiding && p.whichObjectRiding->canJumpFromRideable))) {
        if (!(downKey && (p.checkDropThruCloud() || p.isGrabbing || p.isRidingObject))) {
            if (!p.jump || p.canFudgeJump()) {
                p.jump = true;
                p.jumpspeed = p.jumpstart;
                p.gravity = p.jumpgravity;
                p.isDoubleJumping = false;
                p.canPressDoubleJump = false;
                p.canPressWallJump = false;
            } else if (free && p.skillType == CustomerData::SKILL_DOUBLEJUMP && !p.isDoubleJumping &&
                       p.canPressDoubleJump && !p.didDoubleJump) {
                p.startDoubleJumping();
                p.canPressDoubleJump = false;
                p.didDoubleJump = true;
            } else if (free && p.skillType == CustomerData::SKILL_GLIDE && !p.isGliding && p.canPressDoubleJump &&
                       !p.didDoubleJump) {
                p.startGliding();
                p.canPressDoubleJump = false;
                p.didDoubleJump = true;
            }
            p.duck = false;
            p.isDucking = false;
            p.jumpedFromSlope = p.onSlope;
            p.jumpedFromX = p.x;
            p.jumpedFromY = p.y;
            if (p.isCheesed) {
                p.jumpspeed = -10;
                p.jumpCheesed();
            } else if (p.isInQuicksand) {
                p.jumpspeed = -16;
            } else if (p.isInRiver) {
                p.jumpspeed = -14;
            }
            if (!p.isGliding && !p.isCheesed && !p.isInQuicksand) {
                if (p.whichJumpSound == 1) {
                    gameObj->obf_7C->playSound("jump1");
                    p.whichJumpSound = 2;
                } else {
                    gameObj->obf_7C->playSound("jump2");
                    p.whichJumpSound = 1;
                }
            } else if (p.isCheesed || p.isInQuicksand) {
                gameObj->obf_7C->playSound("jumpcheesed.wav");
            }
        }
        p.isGrabbing = false;
        p.isStartingGrabbing = false;
        p.canPressJump = false;
        p.lastSafeTime = 0;
        if (p.isRidingObject) {
            p.stopGrabRiding();
        }
    } else if (jumpKey && p.jump && p.skillType == CustomerData::SKILL_WALLJUMP && p.checkIfAgainstWall() &&
               !p.isGrabbingObject && p.canPressWallJump &&
               (!p.isRidingObject || (p.whichObjectRiding && p.whichObjectRiding->canJumpFromRideable)) &&
               !p.isCheesed && !p.isInQuicksand) {
        if (!p.checkIfOnLadder(-1) && ((p.facingDir == 1 && p.checkIfAgainstWall(1) && p.isWallSliding) ||
                                       (p.facingDir == -1 && p.checkIfAgainstWall(-1) && p.isWallSliding))) {
            p.startWallJumping();
            if (p.whichJumpSound == 1) {
                gameObj->obf_7C->playSound("jump1");
                p.whichJumpSound = 2;
            } else {
                gameObj->obf_7C->playSound("jump2");
                p.whichJumpSound = 1;
            }
            p.canPressDoubleJump = false;
        }
    } else if (!jumpKey && p.jump && p.jumpspeed < p.shortjump && !p.triggerjump && !p.isDoubleJumping &&
               !p.isWallJumping && !p.isCheesed && !p.isInQuicksand) {
        // Releasing the key early cuts the jump short.
        p.jumpspeed = p.shortjump;
        p.canPressJump = true;
        if (!p.didDoubleJump) {
            p.canPressDoubleJump = true;
        }
        p.lastSafeTime = 0;
    } else if (!jumpKey && !p.jump) {
        p.canPressJump = true;
        p.isDoubleJumping = false;
        p.didDoubleJump = false;
    } else if (!jumpKey && p.jump && !p.didDoubleJump) {
        p.canPressDoubleJump = true;
    }
    if (!jumpKey && p.isGliding) {
        p.isGliding = false;
    }
    if (!p.canPressWallJump && !jumpKey) {
        p.canPressWallJump = true;
    } else if (!p.canArrowWallJump) {
        if (p.lastWallJumpKey == "left" && !keyPressedLeft) {
            p.canArrowWallJump = true;
        } else if (p.lastWallJumpKey == "right" && !keyPressedRight) {
            p.canArrowWallJump = true;
        }
    }
}

// AS: GameControls.keyCheckPlayer
void GameControls::keyCheckPlayer(CustomerChar &p)
{
    const bool action = keyPressedAction;
    const bool left = keyPressedLeft, right = keyPressedRight, down = keyPressedDown;
    p.lastx = p.x;
    p.lasty = p.y;
    p.lastOnSlope = p.onSlope;
    if (p.slopedIntoCeiling == 0) {
        jumpPlayer(p);
    }
    if (down && !p.isClimbingLadder && !p.isCheesed && !p.jump && !p.isCrawling && !left && !right && p.canPressDown) {
        p.startDucking(); // SKELETON: throwing / grabbing objects come first in the original
        p.canPressDown = false;
    } else if (down && p.jump && !p.isClimbingLadder && !p.isCheesed && !p.isGroundPounding && !p.isRidingObject &&
               !p.isRidingElevator && p.canPressDown && p.skillType == CustomerData::SKILL_POUND) {
        p.startPounding();
        p.canPressDown = false;
    } else if (!down || p.isCrawling) {
        if (p.duck && !p.isInsideCrawlGap() && !p.isHit) {
            p.duck = false;
            p.isTurning = false;
            p.isCrawling = false;
        }
        p.canPressDown = true;
    }
    const bool up = keyPressedUp;
    if (!p.isAttacking && !p.isWallSliding && !p.isPushingObject && !p.isSliding && !p.isGliding && !p.isGroundPounding &&
        !p.isCheesed && (!p.jump || p.jumpspeed >= 0)) {
        const double tileWidth = gameObj->obf_71->tileWidth;
        // Grabbing a ladder snaps the player to its side, facing the way they were.
        auto grabLadder = [&](double dir) {
            if (!p.isClimbingLadder) {
                gameObj->obf_7C->playSound("ladder_grab.wav");
            }
            p.isClimbingLadder = true;
            p.climbDir = dir;
            if (p.facingDir == 1) {
                p.x = p.xtile * tileWidth + 12;
            } else {
                p.x = (p.xtile + 1) * tileWidth - 12;
            }
            if (p.jump && p.jumpspeed >= 0) {
                p.jump = false;
            }
        };
        if (up && p.checkIfOnLadder(-1)) {
            grabLadder(-1);
            p.canPressDoubleJump = false;
        } else if (down && (p.checkIfOnLadder(1) || p.checkIfLadderBelow())) {
            if (p.checkIfLadderBelow() && !p.checkIfOnLadder(1)) {
                p.y += 18; // step down onto the ladder below
                p.ytile = std::floor(p.y / tileWidth);
            }
            grabLadder(1);
        } else if (p.isClimbingLadder && !down && !up) {
            p.climbDir = 0;
        }
    }
    if (p.isClimbingLadder && p.climbDir != 0) {
        p.moveChar(0, p.climbDir, 0, false, false, true, p.climbSpeed);
        if (p.isClimbingLadder && !p.checkIfOnLadder(p.climbDir)) {
            // Off the top or bottom: a small hop.
            p.isClimbingLadder = false;
            p.jump = true;
            p.jumpspeed = -12;
        }
    }
    // The left and right branches of the original are the same code mirrored.
    auto walk = [&](double dir) {
        if (p.isInRiver || p.isRidingObject || p.isSliding || p.isCheesed || p.isGroundPounding || p.isClimbingLadder ||
            p.isAttackLunging) {
            p.isWalking = false;
        } else if (p.facingDir == -dir && p.isWallSliding) {
            // Pressing away from the wall jumps off it.
            if (p.checkIfAgainstWall(-dir) && p.canArrowWallJump) {
                p.isWalking = true;
                p.startWallJumping(dir, dir < 0 ? "left" : "right");
                if (p.whichJumpSound == 1) {
                    gameObj->obf_7C->playSound("jump1");
                    p.whichJumpSound = 2;
                } else {
                    gameObj->obf_7C->playSound("jump2");
                    p.whichJumpSound = 1;
                }
            } else {
                p.isWalking = false;
            }
        } else if (p.isWallJumping) {
            p.isWalking = false;
        } else if (p.isDucking && !p.duck) {
            p.walkingDir = dir;
            p.facingDir = dir;
            p.isWalking = false;
        } else if ((p.duck && (p.skillType != CustomerData::SKILL_CRAWL || !p.isCrawling)) || p.isGrabbingObject ||
                   p.isRidingElevator) {
            p.walkingDir = dir;
            p.facingDir = dir;
            p.isWalking = false;
            if (p.duck && p.skillType == CustomerData::SKILL_CRAWL && !p.isCrawling && p.isInsideCrawlGap()) {
                SDL_Log("You slid into a gap, you can crawl now.");
                if (!p.isCrawling) {
                    gameObj->obf_7C->playSound("crawlgap.wav");
                }
                p.isCrawling = true;
            }
        } else if (p.isAttacking && p.facingDir == -dir) {
            // Turning round cancels a swing.
            if (!(p.usingGun() || p.usingPowerTool())) {
                p.cancelAttack();
            }
            p.setSpeed(p.walkspeed);
            p.walkingDir = dir;
            p.facingDir = dir;
            p.moveChar(dir, 0, 0);
        } else if (p.isAttacking && p.facingDir == dir && !p.canDealDamage && !p.jump) {
            // After a hit lands, walking on is held back for postAttackDelayMax frames.
            p.walkingDir = dir;
            p.facingDir = dir;
            p.isWalking = false;
            (dir < 0 ? p.canPressLeft : p.canPressRight) = false;
            p.postAttackDelayTimer = 0;
        } else if (p.jump && p.isWalking) {
            // In the air the speed changes one step per frame towards the new direction.
            if (p.walkingDir == dir) {
                if (p.speed < p.walkspeed) {
                    p.speed += 1;
                    if (p.speed > p.walkspeed) {
                        p.setSpeed(p.walkspeed);
                    }
                }
                if (p.speed > p.walkspeed) {
                    --p.speed;
                }
            } else if (p.walkingDir == -dir) {
                if (p.speed > p.walkspeed * -1) {
                    --p.speed;
                    if (p.speed < p.walkspeed * -1) {
                        p.setSpeed(p.walkspeed * -1);
                    }
                }
            }
            p.facingDir = dir;
            p.moveChar(p.walkingDir, 0, 0);
        } else {
            if (p.walkingDir == -dir && !p.jump && !p.isShoved) {
                p.isTurning = true;
            }
            if (p.speed > p.walkspeed && p.jump) {
                --p.speed;
            } else {
                p.setSpeed(p.walkspeed);
            }
            p.walkingDir = dir;
            p.facingDir = dir;
            p.moveChar(dir, 0, 0);
        }
    };
    if (left && p.canPressLeft && p.slopedIntoCeiling != -1) {
        walk(-1);
    } else if (right && p.canPressRight && p.slopedIntoCeiling != 1) {
        walk(1);
    } else {
        if (p.speed < 0) {
            p.speed *= -1;
            p.walkingDir *= -1;
        }
        if (p.jump && p.isWalking && !p.isGrabbing && !p.isClimbingLadder && !p.isRidingObject && !p.isWallJumping &&
            !p.isSliding && !p.isAttackLunging && p.slopedIntoCeiling == 0 && !p.isCheesed) {
            // Keys released mid-air: keep drifting, slowing down every other frame.
            if (p.speed >= 2) {
                if (p.speed > 2 && std::fmod(gameplayTimer, 2) == 0) {
                    --p.speed;
                }
                if (p.skidSpeed > p.speed) {
                    p.skidSpeed = p.speed;
                }
                p.moveChar(p.walkingDir, 0, 0);
            }
        } else if (!p.jump && !p.isShoved) {
            p.isWalking = false;
        } else if (p.isShoved && !keyPressedRight && !keyPressedLeft) {
            p.isWalking = false;
        }
        if (p.isShoved && p.isAttacking && !p.canDealDamage) {
            p.isShoved = false;
            p.postAttackDelayTimer = 0;
            p.isWalking = false;
            if (p.facingDir == 1) {
                p.canPressRight = false;
                p.canPressLeft = true;
            } else if (p.facingDir == -1) {
                p.canPressRight = true;
                p.canPressLeft = false;
            }
        }
        if (p.isSkidding && !p.isGrabbing && !p.isRidingObject && !p.isClimbingLadder && !p.isShoved && !p.isSliding &&
            p.slopedIntoCeiling == 0 && !p.onMovingTile && !p.isCheesed) {
            // Keys released after a run: slide on with the decaying skid speed.
            p.setSpeed(p.useSkidSpeed);
            p.moveChar(p.walkingDir, 0, 0);
        }
    }
    if (p.isSliding && !p.isGrabbing && !p.isCheesed && !p.isClimbingLadder && !p.isRidingObject && !p.isShoved) {
        p.setSpeed(p.slideSpeed);
        p.moveChar(p.walkingDir, 0, 0);
    }
    if (p.isAttackLunging && !p.isClimbingLadder && !p.isCheesed && !p.isRidingObject && !p.isShoved) {
        p.setSpeed(p.useLungeSpeed);
        p.moveChar(p.lungeDirection, 0, 0);
    }
    if (action) {
        if (p.isCarryingObject) {
            // SKELETON: throwing the carried object (startThrowingObject).
        } else if (p.canAttack && !p.duck && !p.isDucking && !p.isCheesed && !p.isClimbingLadder && !p.isSliding && !p.isGroundPounding &&
                   !p.isWallSliding && p.canPressAction) { // SKELETON: also not grabbing / throwing an object
            p.startAttacking();
            p.canPressAction = false;
        }
    } else if (!p.canPressAction) {
        p.canPressAction = true;
    }
}

// AS: GameControls.travelThroughDoor — the room switch happens when the "out" wipe finishes.
void GameControls::travelThroughDoor(double roomID, double doorID, double xOffset, double yOffset, double dirX,
                                     double dirY)
{
    doorRoomID = roomID;
    doorDoorID = doorID;
    doorXoffset = xOffset;
    doorYoffset = yOffset;
    doorDirX = dirX;
    doorDirY = dirY;
    gameObj->obf_3Y->unsetCameraForceScrolling();
    travelingThroughDoor = true;
    gameObj->obf_71->startTransition("out");
    stopCycle = true;
}

// AS: GameControls.pauseGame
void GameControls::pauseGame()
{
    isPaused = true;
    gameObj->obf_7C->muteSound(false);
    if (gameObj->obf_6Z && gameObj->obf_6Z->interfaceClip) {
        if (engine::MovieClip *paused = gameObj->obf_6Z->interfaceClip->findChild("pausedMC")) {
            paused->setVisible(true);
        }
    }
}

// AS: GameControls.resumeGame
void GameControls::resumeGame(bool keysWereChanged)
{
    isPaused = false;
    if (!gameObj->obf_7C->isMute) {
        gameObj->obf_7C->unmuteSound(false);
    }
    if (keysWereChanged) {
        gameObj->obf_7u->updateTrainingSigns();
    }
    if (gameObj->obf_6Z && gameObj->obf_6Z->interfaceClip) {
        if (engine::MovieClip *paused = gameObj->obf_6Z->interfaceClip->findChild("pausedMC")) {
            paused->setVisible(false);
        }
    }
}

} // namespace papaGame::events
