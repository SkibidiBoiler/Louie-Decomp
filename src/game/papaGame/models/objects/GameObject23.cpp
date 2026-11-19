#include "game/papaGame/models/objects/GameObject23.h"

namespace papaGame::models::objects {

// AS: GameObject23.defineVars
void GameObject23::defineVars()
{
    GameObject21::defineVars();
    sheetname = "object_crackerblock3";
    sheetIsBitmap = false;
    objectname = "Cracker Block 3";
    type = 23;
    spritewidth = 64;
    spriteheight = 32;
    spriteCenterX = 32;
    spriteCenterY = 16;
    collRect = {-20, -20, 72, 40};
    solidTilesRect = {0, 0, 2, 1};
}

} // namespace papaGame::models::objects
