#include "game/papaGame/models/objects/GameObject16.h"

namespace papaGame::models::objects {

// AS: GameObject16.defineVars
void GameObject16::defineVars()
{
    GameObject15::defineVars();
    sheetname = "object_offberry";
    objectname = "Off Berry";
    sheetIsBitmap = false;
    type = 16;
}

} // namespace papaGame::models::objects
