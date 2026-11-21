#include "game/papaGame/models/objects/GameObject32.h"

namespace papaGame::models::objects {

// AS: GameObject32.defineVars
void GameObject32::defineVars()
{
    GameObject27::defineVars();
    sheetname = "object_rainbowsauce";
    sheetIsBitmap = false;
    objectname = "Rainbow Sauce";
    type = 32;
    tilingHasBottomCap = true;
    tilingBottomCapBlitOffsetY = 128;
    sheetHeight = 3;
}

} // namespace papaGame::models::objects
