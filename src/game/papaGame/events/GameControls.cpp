#include "game/papaGame/events/GameControls.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::events {

using models::characters::CustomerChar;

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
    ++gameplayTimer;
    ++currentLevelTimer;
    // SKELETON: time bonus, being shoved.
    if (!stopControls) {
        keyCheckPlayer(p);
    }
    if (p.jump) {
        p.duck = false;
        p.isDucking = false;
        p.isCrawling = false;
        if (p.jumpspeed < 0) {
            p.isClimbingLadder = false;
        }
        p.jumpChar();
    }
    if (wasJumping && !p.jump && !p.isWalking && p.skidSpeed != 0) {
        p.skidSpeed = 0;
    }
    if (!p.isWalking && p.jump && p.skidSpeed != 0) {
        p.skidSpeed = 0;
    }
    if (wasJumping && !p.jump) {
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
    p.updateObject();
    // SKELETON: items, bullets, enemies, effects and objects are processed here; then adjustCamera, the HUD
    // timer and redrawScreen (the port redraws in its render pass).
}

// AS: GameControls.jumpPlayer (without the skill branches: double jump, glide, wall jump)
void GameControls::jumpPlayer(CustomerChar &p)
{
    const bool jumpKey = keyPressedJump;
    const bool downKey = keyPressedDown;
    const bool canStart = !p.jump || p.canFudgeJump();
    if (jumpKey && canStart && p.canPressJump && (!p.duck || !p.canCrawl || !p.isCrawling) && !p.isSliding &&
        !p.isClimbingLadder) {
        if (!(downKey && p.checkDropThruCloud())) {
            p.jump = true;
            p.jumpspeed = p.jumpstart;
            p.gravity = p.jumpgravity;
            p.isDoubleJumping = false;
            p.canPressDoubleJump = false;
            p.canPressWallJump = false;
            p.duck = false;
            p.isDucking = false;
            p.jumpedFromSlope = p.onSlope;
            p.jumpedFromX = p.x;
            p.jumpedFromY = p.y;
            // SKELETON: jump1 / jump2 sounds (whichJumpSound alternates).
            p.whichJumpSound = p.whichJumpSound == 1 ? 2 : 1;
        }
        p.canPressJump = false;
        p.lastSafeTime = 0;
    } else if (!jumpKey && p.jump && p.jumpspeed < p.shortjump && !p.triggerjump && !p.isDoubleJumping) {
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
    if (!p.canPressWallJump && !jumpKey) {
        p.canPressWallJump = true;
    }
}

// AS: GameControls.keyCheckPlayer
void GameControls::keyCheckPlayer(CustomerChar &p)
{
    const bool left = keyPressedLeft, right = keyPressedRight, down = keyPressedDown;
    p.lastx = p.x;
    p.lasty = p.y;
    p.lastOnSlope = p.onSlope;
    if (p.slopedIntoCeiling == 0) {
        jumpPlayer(p);
    }
    if (down && !p.isClimbingLadder && !p.jump && !p.isCrawling && !left && !right && p.canPressDown) {
        p.startDucking(); // SKELETON: throwing / grabbing objects come first in the original
        p.canPressDown = false;
    } else if (!down || p.isCrawling) {
        if (p.duck) {
            p.duck = false;
            p.isTurning = false;
            p.isCrawling = false;
        }
        p.canPressDown = true;
    }
    // SKELETON: ladders (keyPressedUp / Down next to a ladder).
    auto walk = [&](double dir) {
        if (p.isSliding || p.isClimbingLadder) {
            p.isWalking = false;
        } else if (p.isDucking && !p.duck) {
            p.walkingDir = dir;
            p.facingDir = dir;
            p.isWalking = false;
        } else if (p.duck && (!p.canCrawl || !p.isCrawling)) {
            p.walkingDir = dir;
            p.facingDir = dir;
            p.isWalking = false;
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
        if (p.jump && p.isWalking && !p.isClimbingLadder && !p.isSliding && p.slopedIntoCeiling == 0) {
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
        }
        // SKELETON: skidding (isSkidding / useSkidSpeed) after landing from a run.
    }
    // SKELETON: sliding, attack lunges, attacking (keyPressedAction) and throwing.
}

} // namespace papaGame::events
