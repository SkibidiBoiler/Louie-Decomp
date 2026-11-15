#include "game/papaGame/models/characters/CustomerChar.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::characters {

CustomerChar::CustomerChar(obf_9G::obf_4R *gameObj_) : Character(gameObj_)
{
    defineVars(); // AS: Character's constructor calls defineVars()
}

// AS: CustomerChar.defineVars (movement values; collision rectangles, health and skills come later)
void CustomerChar::defineVars()
{
    height = 15;
    width = 15;
    spritewidth = 74;
    spriteheight = 60;
    heightmultiplier = 5;
    widthmultiplier = 1;
    speed = 8;
    walkspeed = 8;
    jumpspeed = 0;
    jumpstart = -23;
    gravity = 2;
    normalgravity = 2;
    jumpgravity = 2;
    shouldCheckCeilingSlopes = true;
    if (const data::CustomerDataFile *c = gameObj->obf_3A->getCustomerData(gameObj->obf_51->selectedCharacter)) {
        canCrawl = c->skillType == data::CustomerData::SKILL_CRAWL;
        if (canCrawl) {
            playerheight = 50;
        }
    }
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
    isWalking = false;
    isDead = false;
    prevX = x;
    prevY = y;
    // SKELETON: the rest of startupPlayer (state flags, effects, projectiles) comes with the controls.
}

void CustomerChar::advanceStandCycle()
{
    cycleFrame = cycleFrame + 1 >= static_cast<double>(standCycle.size()) ? 0 : cycleFrame + 1;
    animationFrame = standCycle[static_cast<size_t>(cycleFrame)];
    footAnimationFrame = footStandCycle[static_cast<size_t>(cycleFrame)];
}

} // namespace papaGame::models::characters
