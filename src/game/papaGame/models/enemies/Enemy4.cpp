#include "game/papaGame/models/enemies/Enemy4.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::enemies {

// AS: Enemy4.defineVars
void Enemy4::defineVars()
{
    Enemy3::defineVars();
    type = 4;
    sheetname = "enemysheet_goldonion";
    sheetIsBitmap = false;
    saveKillAsID = 2;
    enemyCategory = "onion";
}

// AS: Enemy4.killMe — it drops its gold helmet.
void Enemy4::killMe(double dir)
{
    if (!isDead) {
        // AS: addItem(xtile, ytile, "GoldHelmet", 1) — 1 is passed as whichScreen.
        gameObj->obf_3Z->addItem(xtile, ytile, as3::Value::str("GoldHelmet"), 1);
    }
    Enemy3::killMe(dir);
}

} // namespace papaGame::models::enemies
