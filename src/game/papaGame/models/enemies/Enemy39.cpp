#include "game/papaGame/models/enemies/Enemy39.h"

namespace papaGame::models::enemies {

// AS: Enemy39.defineVars
void Enemy39::defineVars()
{
    Enemy38::defineVars();
    type = 39;
    sheetname = "enemysheet_RocketMustard";
    sheetIsBitmap = false;
    canHitBack = true;
    rocketType = "RocketMustard";
}

} // namespace papaGame::models::enemies
