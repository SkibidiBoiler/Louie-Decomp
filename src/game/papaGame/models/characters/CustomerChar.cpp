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

// AS: PlayerChar.canFudgeJump — a few frames after walking off a ledge a jump still counts.
bool CustomerChar::canFudgeJump() const
{
    return gameObj->obf_8o->gameplayTimer < lastSafeTime + lastSafeTimeBuffer && !duck && !isCrawling && !isDucking &&
           (!jump || jumpspeed >= 0);
}

// AS: PlayerChar.startDucking
void CustomerChar::startDucking()
{
    // SKELETON: on a slope the original starts sliding (startSliding) instead.
    duck = true;
    isDucking = true;
    isTurning = false;
}

void CustomerChar::updateObject()
{
    // Cycles: [name, ?, ?, ?, loop mode (0 loops, -1 plays once, 2/4 hold the last frame), frames].
    static const std::vector<double> stand = {0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3};
    static const std::vector<double> run = {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
    static const std::vector<double> jumpUp = {26, 27, 28};
    static const std::vector<double> fall = {29, 30, 31};
    static const std::vector<double> turnaround = {23, 24, 25, 25, 25};
    static const std::vector<double> duckDown = {32, 33, 34, 35, 36};
    if (!jump) {
        lastSafeTime = gameObj->obf_8o->gameplayTimer; // (CustomerChar sets it while standing on ground)
    }
    const std::vector<double> *cycle = &stand;
    bool loops = true;
    if (jump) {
        cycle = jumpspeed < 0 ? &jumpUp : &fall;
        loops = false;
    } else if (duck) {
        cycle = &duckDown;
        loops = false;
    } else if (isTurning) {
        cycle = &turnaround;
        loops = false;
    } else if (isWalking) {
        cycle = &run;
    }
    if (cycle != currentCycle) {
        currentCycle = cycle;
        cycleFrame = 0;
    } else if (cycleFrame + 1 < static_cast<double>(cycle->size())) {
        ++cycleFrame;
    } else if (loops) {
        cycleFrame = 0;
    } else if (cycle == &turnaround) {
        isTurning = false; // plays once
    }
    animationFrame = (*cycle)[static_cast<size_t>(cycleFrame)];
    footAnimationFrame = animationFrame; // the foot cycles use the same frame numbers for these states
}

} // namespace papaGame::models::characters
