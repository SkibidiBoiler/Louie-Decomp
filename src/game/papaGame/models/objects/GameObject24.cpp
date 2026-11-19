#include "game/papaGame/models/objects/GameObject24.h"

namespace papaGame::models::objects {

// AS: GameObject24.defineVars
void GameObject24::defineVars()
{
    GameObject21::defineVars();
    sheetname = "object_crackerblock4";
    sheetIsBitmap = false;
    objectname = "Cracker Block 4";
    type = 24;
    spritewidth = 64;
    spriteheight = 64;
    spriteCenterX = 32;
    spriteCenterY = 32;
    collRect = {-20, -20, 72, 72};
    solidTilesRect = {0, 0, 2, 2};
}

} // namespace papaGame::models::objects
