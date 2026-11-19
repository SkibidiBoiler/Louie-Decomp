#include "game/papaGame/models/enemies/Enemy10.h"

namespace papaGame::models::enemies {

// AS: Enemy10.defineVars
void Enemy10::defineVars()
{
    Enemy9::defineVars();
    type = 10;
    sheetname = "enemysheet_partysubmiddle";
    sheetIsBitmap = false;
    saveKillAsID = 11;
    enemyCategory = "partysub";
}

} // namespace papaGame::models::enemies
