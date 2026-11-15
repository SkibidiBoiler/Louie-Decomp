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

    // Port: remember the position before a logic step (interpolation), then the SKELETON stand cycle.
    void savePrevious() { prevX = x; prevY = y; }
    void advanceStandCycle();

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
