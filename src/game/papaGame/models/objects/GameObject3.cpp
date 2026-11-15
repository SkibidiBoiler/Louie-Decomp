#include "game/papaGame/models/objects/GameObject3.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::objects {

// AS: GameObject3 constructor — the ladder's tiles, from its bottom tile up.
void GameObject3::init(double xtile_, double ytile_, double id_, double whichScreen_, double screenObjID_,
                       const std::vector<as3::Value> &extraParams)
{
    GameObject::init(xtile_, ytile_, id_, whichScreen_, screenObjID_, extraParams);
    managers::MapManager &map = *gameObj->obf_4i;
    const double top = ytile - tilingHeight + 1;
    for (int i = 0; i < tilingHeight; ++i) {
        map.setProperty(xtile, top + i, "ladder");
    }
}

// AS: GameObject3.defineVars
void GameObject3::defineVars()
{
    sheetname = "ladder_sheet";
    objectname = "Ladder";
    type = 3;
    spritewidth = 32;
    spriteheight = 32;
    spriteCenterX = 16;
    spriteCenterY = 16;
    spriteTargetX = 16;
    spriteTargetY = 16;
    sheetWidth = 3;
    sheetHeight = 1;
    collRect = {-16, -16, 32, 32};
    isActivated = true;
    foreground = true;
    broadcastStyle = 1;
    activeCycleFrames = cycleType1("active", 1, 0, 0, 0, 0);
    activatingCycleFrames = cycleType2("activating", 1, 0, -1, {99});
    deactivatingCycleFrames = cycleType2("deactivating", 1, 0, -1, {99});
    inactiveCycleFrames = cycleType1("inactive", 1, 0, 99, 99, 99);
    isLadder = true;
    topCapFrame = 2;
    tilingFrame = 0;
    bottomCapFrame = 1;
}

// AS: GameObject3.checkOnScreen
bool GameObject3::checkOnScreen()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 4 && xtile < d.currentXtileRight + 4;
}

// AS: GameObject3.setupParams — [height in tiles]
void GameObject3::setupParams(const std::vector<as3::Value> &params)
{
    if (params.size() > 0) {
        tilingHeight = params[0].asNumber();
    }
}

// AS: GameObject3.updateObject (the activated branch is empty)
void GameObject3::updateObject()
{
    updateSprite();
}

} // namespace papaGame::models::objects
