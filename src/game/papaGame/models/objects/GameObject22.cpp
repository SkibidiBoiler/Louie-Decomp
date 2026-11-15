#include "game/papaGame/models/objects/GameObject22.h"

namespace papaGame::models::objects {

// AS: GameObject22.defineVars
void GameObject22::defineVars()
{
    GameObject21::defineVars();
    sheetname = "object_crackerblock2";
    sheetIsBitmap = false;
    objectname = "Cracker Block 2";
    type = 22;
    spritewidth = 32;
    spriteheight = 64;
    spriteCenterX = 16;
    spriteCenterY = 32;
    collRect = {-20, -20, 40, 72};
    solidTilesRect = {0, 0, 1, 2};
}

} // namespace papaGame::models::objects
