#include "game/papaGame/models/objects/GameObject20.h"

namespace papaGame::models::objects {

// AS: GameObject20 constructor — the player ends on the cage's other side. (Decompiled through §§pop; read from the
// P-code, whose opaque predicates test registers that are never set.)
void GameObject20::init(double xtile_, double ytile_, double id_, double whichScreen_, double screenObjID_,
                        const std::vector<as3::Value> &extraParams)
{
    GameObject19::init(xtile_, ytile_, id_, whichScreen_, screenObjID_, extraParams);
    playerEndX = x - spriteTargetX + spritewidth + 32;
}

// AS: GameObject20.defineVars
void GameObject20::defineVars()
{
    GameObject19::defineVars();
    sheetname = "object_cageflipped";
    sheetIsBitmap = false;
    objectname = "Cage Flipped";
    type = 20;
    facingDir = 1;
    fx_offset = 40;
}

} // namespace papaGame::models::objects
