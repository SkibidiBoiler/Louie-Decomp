#include "game/papaGame/managers/MapManager.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::managers {

// AS: MapManager.isWithinBounds
bool MapManager::isWithinBounds(double x, double y) const
{
    const auto &tiles = gameObj->obf_AQ->currentScreenData->tileArray;
    return x >= 0 && y >= 0 && x < static_cast<double>(tiles[0].size()) && y < static_cast<double>(tiles.size());
}

// AS: MapManager.getTileProperty
double MapManager::getTileProperty(double x, double y, std::string_view property) const
{
    const data::DataManager &dm = *gameObj->obf_AQ;
    if (!isWithinBounds(x, y)) {
        return 0;
    }
    const auto &cell = dm.currentScreenData->tileArray[static_cast<size_t>(y)][static_cast<size_t>(x)];
    if (property == "collision" || property == "naturalcollision" || property == "grab" || property == "ladder") {
        return dm.getPropValue(property, cell);
    }
    if (property == "thrublock") {
        return dm.getPropValue("thru", cell);
    }
    if (property == "slope" || property == "solidslope" || property == "ceilingslope" ||
        property == "ceilingslopereverse") {
        double v = dm.getPropValue(property, cell);
        // A slope value on a tile that also collides counts as no slope.
        if (v != 0 && v > 0 && dm.getPropValue("collision", cell) != 0) {
            v = 0;
        }
        return v;
    }
    // SKELETON: "door" (ScreenData.checkForDoorID + WorldData.getDoorArray) comes with doors.
    return 0;
}

} // namespace papaGame::managers
