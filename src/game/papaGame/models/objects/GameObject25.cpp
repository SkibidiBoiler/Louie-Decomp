#include "game/papaGame/models/objects/GameObject25.h"

namespace papaGame::models::objects {

// AS: GameObject25.defineVars
void GameObject25::defineVars()
{
    GameObject21::defineVars();
    sheetname = "object_crackerblock5";
    sheetIsBitmap = false;
    objectname = "Cracker Block 5";
    type = 25;
    spritewidth = 64;
    spriteheight = 96;
    spriteCenterX = 32;
    spriteCenterY = 48;
    collRect = {-20, -20, 72, 104};
    solidTilesRect = {0, 0, 2, 3};
}

} // namespace papaGame::models::objects
