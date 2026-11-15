#pragma once
// AS: papaGame.models.characters.CustomerChar (extends PlayerChar extends Character) — the player.
// SKELETON: PlayerChar is folded in for now; damage, items, doors, skills and finishing the level come later.
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "game/papaGame/managers/CharacterBlitter.h"
#include "game/papaGame/models/Character.h"
#include "game/papaGame/models/GameObject.h"

namespace papaGame::data { class CustomerDataFile; }

namespace papaGame::models::characters {

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

    void updateObject() override;
    void fallChar() override;
    void setSpeed(double v) { speed = v; }  // AS: PlayerChar.setSpeed
    bool canFudgeJump() const;              // AS: PlayerChar.canFudgeJump
    void startDucking();                    // AS: PlayerChar.startDucking (no sliding on slopes yet)
    bool checkIfOnLadder(double dir);       // AS: PlayerChar.checkIfOnLadder
    void checkIfOnDoor();                   // AS: PlayerChar.checkIfOnDoor
    void checkGoingThroughDoor(double dx, double dy) override;
    void enterDoor(double tileX, double tileY);
    void fellOffLevel(double maxY) override;
    // Damage (AS: PlayerChar).
    double getNormalAttackPower() const { return statAttack; }
    void clearCombo();
    bool canGetHit() const;
    void hurtPlayer(double dir, double damage, bool forced = false);
    void shovePlayer(double dir, double amount = 4);
    void restartPlayer(double lives = -1);
    void updateStunTime();
    void getHeart();                // AS: PlayerChar.getHeart
    void checkForCollectingItems(); // AS: PlayerChar.checkForCollectingItems
    void checkForGrabbingTile();    // AS: CustomerChar.checkForGrabbingTile
    void finishLevel(const std::string &how = "cage"); // AS: PlayerChar.finishLevel
    void dropWeapon() {} // AS: PlayerChar.dropWeapon (empty)
    bool checkForHittingEnemies(bool justChecking = false); // AS: CustomerChar.checkForHittingEnemies
    bool checkForHittingObjects();                          // AS: CustomerChar.checkForHittingObjects
    void checkForPoundingObjects();                         // AS: CustomerChar.checkForPoundingObjects
    void addToCombo(double amount = 1); // AS: PlayerChar.addToCombo
    void updateComboTimer();            // AS: PlayerChar.updateComboTimer

    // Animation.
    const AnimationCycle &grabAnimationCycle() override;
    const AnimationCycle &grabFootAnimationCycle();
    void updateSprite() override;
    void footSpriteChangeFrame(double step, const AnimationCycle &cycle, double cycleType);
    void adjustFrameSync(const std::string &newName, const std::string &oldName, double firstFrame,
                         const AnimationCycle &cycle) override;
    void adjustFootFrameSync(const std::string &newName, const std::string &oldName, double firstFrame,
                             const AnimationCycle &cycle);
    void endAnimationCycle() override;

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
    bool isDucking = false, isShoved = false, isSkidding = false;
    bool isGone = false, isGrabbing = false, isRidingObject = false;
    bool isFinishingLevel = false, isAttacking = false, canAttack = true, canDealDamage = true;
    bool isAttackLunging = false, wasJumping = false, wasTurning = false;
    bool isFinishPushing = false, isPushingObject = false, isGroundPounding = false, isWallJumping = false;
    bool isRidingElevator = false;
    double climbDir = 1, climbSpeed = 5;
    bool waitToEnterDoor = false;
    double doorDirX = 0, doorDirY = 0, doorXtile = 0, doorYtile = 0;
    double skidSpeed = 0, useSkidSpeed = 0, maxSkidSpeed = 8, skidTimer = 0, skidTimerInterval = 2;
    double skidDecayTimer = 0, skidDecayTimerInterval = 1, skidDecayAmount = 1;
    double lastSafeTime = 0, lastSafeTimeBuffer = 6, whichJumpSound = 1, lastSafeX = 0, lastSafeY = 0;
    double lungeSpeed = 0, useLungeSpeed = 0, lungeTimer = 0, lungeTimerInterval = 1, lungeDirection = 1;
    double baseWalkSpeed = 8, playerLastX = 0;
    double attackVariation = 1, postAttackDelayTimer = 0, postAttackDelayMax = 1;
    double startAttackFrame = 0, endAttackFrame = 4, repeatAttackFrame = 6;
    double reloadTimer = 0, reloadTimerMax = 1, bulletTimer = 0, bulletsFired = 0, bulletsPerRound = 1;
    double comboTally = 0, comboTimer = 0, comboTimerMax = 90;
    bool wasPushing = false;
    bool isFinalBlow = false, isInvincible = false, isStartingGrabbing = false, canAutoWallJump = false;
    bool isCheesed = false, isPausingAfterMiniboss = false, isHiding = false, fellOffTrain = false;
    bool hitByBlast = false, hasPowerup = false;
    double statAttack = 1, invincibleTimer = 0, invincibleFXID = -1, shoveTimer = 0, shoveTimerMax = 6;
    double defaultHealth = 3;
    std::vector<double> stunStarIDs;
    double finishTargetX = 0, finishJumps = 0, finishJumpsMax = 4, finishCustomerFXIndex = -1;
    std::string finishType = "cage";
    double finishWaitTimer = 0, finishWaitTimerMax = 20, finishPortalID = -1;
    double ridingObjectDirX = 0;
    double grabTileYOffset = 0;
    GameObject *whichObjectRiding = nullptr;
    bool finishAddedPortal = false;
    Rect standCollRect, jumpCollRect, duckCollRect, stomp_collRect, pound_collRect;
    std::string hasWeaponType = "unknown";
    Rect attackRect;

    std::unique_ptr<managers::CharacterBlitter> blitter;
    std::string footCycleName = "none";
    double footAnimationFrame = 0, footCycleFrame = 0;

private:
    const data::CustomerDataFile *playerData() const;
    std::string slideCycleName(const std::string &customerName) const;

    CycleTable cycles_; // by field name, e.g. "runCycleFrames"
    std::vector<double> torsoOffsetsX_, torsoOffsetsY_;
    Rect melee_attackRect, swing_attackRect, whip_attackRect, other_attackRect;
    std::vector<double> melee_frameRange, swing_frameRange, whip_frameRange, other_frameRange;
};

} // namespace papaGame::models::characters
