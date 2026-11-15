#pragma once
// AS: papaGame.models.characters.CustomerChar (extends PlayerChar extends Character) — the player.
// SKELETON: only what is needed to show the player standing; Character/PlayerChar movement, collisions and
// the full animation state machine come next.
#include <memory>
#include <vector>

#include "game/papaGame/managers/CharacterBlitter.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::models::characters {

class CustomerChar {
public:
    explicit CustomerChar(obf_9G::obf_4R *gameObj);

    // AS: PlayerChar.startupPlayer (position part) / CustomerChar.startupPlayer
    void startupPlayer(double tileX, double tileY, bool newLevel = false, bool keepFacing = false);
    double getTorsoOffsetX() const { return 0; } // AS: CustomerChar.getTorsoOffsetX (0 unless attacking)
    double getTorsoOffsetY() const { return 0; } // AS: CustomerChar.getTorsoOffsetY

    // SKELETON: one logic frame of the stand cycle (Character.animate is not translated yet).
    void advanceStandCycle();

    obf_9G::obf_4R *gameObj = nullptr;
    std::unique_ptr<managers::CharacterBlitter> blitter;
    double x = 0, y = 0;
    double prevX = 0, prevY = 0; // port: interpolation
    double xtile = 0, ytile = 0;
    double facingDir = -1;
    double walkingDir = 1;
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
