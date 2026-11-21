#include "game/papaGame/models/enemies/Enemy11.h"

namespace papaGame::models::enemies {

// AS: Enemy11.defineVars
void Enemy11::defineVars()
{
    Enemy9::defineVars();
    type = 11;
    sheetname = "enemysheet_partysubfront";
    sheetIsBitmap = false;
    saveKillAsID = 11;
    enemyCategory = "partysub";
}

} // namespace papaGame::models::enemies
