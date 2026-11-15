#pragma once
// AS: papaGame.models.characters.CustomerChar (extends PlayerChar extends Character) — the player.
// SKELETON: PlayerChar is folded in for now (startupPlayer only); controls, the animation state machine,
// attacks, damage and doors come next.
#include <memory>
#include <vector>

#include "game/papaGame/managers/CharacterBlitter.h"
#include "game/papaGame/models/Character.h"

namespace papaGame::models::characters {

class CustomerChar : public Character {
public:
    explicit CustomerChar(obf_9G::obf_4R *gameObj);

    void defineVars() override;
    // AS: PlayerChar.startupPlayer (position part) / CustomerChar.startupPlayer
    void startupPlayer(double tileX, double tileY, bool newLevel = false, bool keepFacing = false);
    double getTorsoOffsetX() const { return 0; } // AS: CustomerChar.getTorsoOffsetX (0 unless attacking)
    double getTorsoOffsetY() const { return 0; } // AS: CustomerChar.getTorsoOffsetY

    bool isPlayer() const override { return true; }

    // Port: remember the position before a logic step (interpolation).
    void savePrevious() { prevX = x; prevY = y; }
    // AS: CustomerChar.updateObject — SKELETON: picks the stand/run/jump/fall/turn/duck cycle; the original
    // state machine (attacks, hurt, skills, sounds) is not translated yet.
    void updateObject();
    void setSpeed(double v) { speed = v; }  // AS: PlayerChar.setSpeed
    bool canFudgeJump() const;              // AS: PlayerChar.canFudgeJump
    void startDucking();                    // AS: PlayerChar.startDucking (no sliding on slopes yet)

    // PlayerChar / CustomerChar fields used by GameControls.
    bool canPressJump = true, canPressAction = true, canPressLeft = true, canPressRight = true, canPressDown = true;
    bool canPressDoubleJump = false, canPressWallJump = true, didDoubleJump = false, isDoubleJumping = false;
    bool isDucking = false, isTurning = false, isShoved = false;
    double skidSpeed = 0, lastSafeTime = 0, lastSafeTimeBuffer = 6, whichJumpSound = 1;
    const std::vector<double> *currentCycle = nullptr;

    std::unique_ptr<managers::CharacterBlitter> blitter;
    double prevX = 0, prevY = 0; // port: interpolation
    double animationFrame = 0;
    double footAnimationFrame = 0;
    double cycleFrame = 0;
    bool isStunned = false;
    bool isAttacking = false;

    // AS: CustomerChar.standCycleFrames / foot_standCycleFrames ["stand", 2, 1, 0, 0, [frames]]
    std::vector<double> standCycle = {0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3};
    std::vector<double> footStandCycle = {0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3};
};

} // namespace papaGame::models::characters
