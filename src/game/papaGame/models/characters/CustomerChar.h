#pragma once
// AS: papaGame.models.characters.CustomerChar (extends PlayerChar extends Character) — the player.
// SKELETON: PlayerChar is folded in for now; damage, items, doors, skills and finishing the level come later.
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "game/papaGame/managers/CharacterBlitter.h"
#include "game/papaGame/models/Character.h"

namespace papaGame::data { class CustomerDataFile; }

namespace papaGame::models::characters {

// One `...CycleFrames` array: [name, type, blitStyle, randomize, loopFrame, frames] (type 2), or
// [name, type, blitStyle, randomize, loopFrame, firstFrame, lastFrame] (type 1).
struct AnimationCycle {
    std::string name;
    double type = 2;
    double blitStyle = 1;
    double randomize = 0;
    double loopFrame = 0; // -1: play once and call endAnimationCycle
    std::vector<double> frames;
    double firstFrame = 0, lastFrame = 0;
};

// A flash.geom.Rectangle.
struct Rect {
    double x = 0, y = 0, width = 0, height = 0;
};

class CustomerChar : public Character {
public:
    explicit CustomerChar(obf_9G::obf_4R *gameObj);

    void defineVars() override;
    // Port: the class's constant tables (animation cycles, torso offsets, attack rectangles), which the
    // importer extracts from the game into customerchar.json.
    bool loadTables(const std::string &path);
    // AS: PlayerChar.startupPlayer (position part) / CustomerChar.startupPlayer
    void startupPlayer(double tileX, double tileY, bool newLevel = false, bool keepFacing = false);
    double getTorsoOffsetX() const;
    double getTorsoOffsetY() const;

    bool isPlayer() const override { return true; }

    // Port: remember the position before a logic step (interpolation).
    void savePrevious() { prevX = x; prevY = y; }
    void updateObject();
    void fallChar() override;
    void setSpeed(double v) { speed = v; }  // AS: PlayerChar.setSpeed
    bool canFudgeJump() const;              // AS: PlayerChar.canFudgeJump
    void startDucking();                    // AS: PlayerChar.startDucking (no sliding on slopes yet)

    // Animation.
    const AnimationCycle &grabAnimationCycle();
    const AnimationCycle &grabFootAnimationCycle();
    void updateSprite();
    void spriteChangeFrame(double step, const AnimationCycle &cycle, double type);
    void footSpriteChangeFrame(double step, const AnimationCycle &cycle, double type);
    void adjustFrameSync(const std::string &newName, const std::string &oldName, double firstFrame,
                         const AnimationCycle &cycle);
    void adjustFootFrameSync(const std::string &newName, const std::string &oldName, double firstFrame,
                             const AnimationCycle &cycle);
    void endAnimationCycle();

    // Attacks.
    void checkIfAttacking();
    void startAttacking();
    void setupAttackRect();
    void cancelAttack();
    bool usingGun() const;
    bool usingPowerTool() const;

    // PlayerChar / CustomerChar fields used by GameControls.
    bool canPressJump = true, canPressAction = true, canPressLeft = true, canPressRight = true, canPressDown = true;
    bool canPressDoubleJump = false, canPressWallJump = true, didDoubleJump = false, isDoubleJumping = false;
    bool isDucking = false, isTurning = false, isShoved = false, isSkidding = false;
    bool isGone = false, isHit = false, isStunned = false, isGrabbing = false, isRidingObject = false;
    bool isFinishingLevel = false, isAttacking = false, canAttack = true, canDealDamage = true;
    bool isAttackLunging = false, wasJumping = false, wasTurning = false;
    bool isFinishPushing = false, isPushingObject = false, isGroundPounding = false, isWallJumping = false;
    bool isRidingElevator = false;
    double climbDir = 1;
    double skidSpeed = 0, useSkidSpeed = 0, maxSkidSpeed = 8, skidTimer = 0, skidTimerInterval = 2;
    double skidDecayTimer = 0, skidDecayTimerInterval = 1, skidDecayAmount = 1;
    double lastSafeTime = 0, lastSafeTimeBuffer = 6, whichJumpSound = 1, lastSafeX = 0, lastSafeY = 0;
    double stunStartTime = 0, stunDuration = 90;
    double lungeSpeed = 0, useLungeSpeed = 0, lungeTimer = 0, lungeTimerInterval = 1, lungeDirection = 1;
    double baseWalkSpeed = 8, playerLastX = 0;
    double attackVariation = 1, postAttackDelayTimer = 0, postAttackDelayMax = 1;
    double startAttackFrame = 0, endAttackFrame = 4, repeatAttackFrame = 6;
    double reloadTimer = 0, reloadTimerMax = 1, bulletTimer = 0, bulletsFired = 0, bulletsPerRound = 1;
    std::string hasWeaponType = "unknown";
    Rect attackRect;

    std::unique_ptr<managers::CharacterBlitter> blitter;
    double prevX = 0, prevY = 0; // port: interpolation
    std::string cycleName, footCycleName = "none";
    double animationFrame = 0, cycleFrame = 0, animationBlitStyle = 1;
    double footAnimationFrame = 0, footCycleFrame = 0;

private:
    const AnimationCycle &cycleFrames(const std::string &field) const;
    const data::CustomerDataFile *playerData() const;
    std::string slideCycleName(const std::string &customerName) const;

    std::map<std::string, AnimationCycle> cycles_; // by field name, e.g. "runCycleFrames"
    std::vector<double> torsoOffsetsX_, torsoOffsetsY_;
    Rect melee_attackRect, swing_attackRect, whip_attackRect, other_attackRect;
    std::vector<double> melee_frameRange, swing_frameRange, whip_frameRange, other_frameRange;
};

} // namespace papaGame::models::characters
