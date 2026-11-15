#include "game/papaGame/models/characters/CustomerChar.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::characters {

CustomerChar::CustomerChar(obf_9G::obf_4R *gameObj_) : gameObj(gameObj_)
{
}

// AS: PlayerChar.startupPlayer
void CustomerChar::startupPlayer(double tileX, double tileY, bool /*newLevel*/, bool keepFacing)
{
    const display::GameDisplay &display = *gameObj->obf_71;
    x = tileX * display.tileWidth + display.tileWidth / 2;
    y = tileY * display.tileWidth + display.tileWidth / 2;
    xtile = tileX;
    ytile = tileY;
    if (!keepFacing) {
        facingDir = 1;
        walkingDir = 1;
    }
    prevX = x;
    prevY = y;
    // SKELETON: the rest of startupPlayer (state flags, effects, projectiles) comes with the movement code.
}

void CustomerChar::advanceStandCycle()
{
    prevX = x;
    prevY = y;
    cycleFrame = cycleFrame + 1 >= static_cast<double>(standCycle.size()) ? 0 : cycleFrame + 1;
    animationFrame = standCycle[static_cast<size_t>(cycleFrame)];
    footAnimationFrame = footStandCycle[static_cast<size_t>(cycleFrame)];
}

} // namespace papaGame::models::characters
