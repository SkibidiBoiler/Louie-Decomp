#include "game/papaGame/managers/MapManager.h"

#include <algorithm>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::managers {

// AS: MapManager.isWithinBounds
bool MapManager::isWithinBounds(double x, double y) const
{
    const auto &tiles = gameObj->obf_AQ->currentScreenData->tileArray;
    return x >= 0 && y >= 0 && x < static_cast<double>(tiles[0].size()) && y < static_cast<double>(tiles.size());
}

// AS: MapManager.setProperty — objects (ladders, blocks) mark tiles by adding a special tile number to the cell.
namespace {

// The marker tile number a property adds to a cell (0 for an unknown property).
double propertyTile(const data::DataManager &dm, std::string_view property)
{
    if (property == "collision") {
        return dm.COLLISION_TILE_NUMBER;
    }
    if (property == "slope") {
        return dm.SLOPE_TILE_NUMBER;
    }
    if (property == "thrublock") {
        return dm.THRUBLOCK_TILE_NUMBER;
    }
    if (property == "grab") {
        return dm.GRAB_TILE_NUMBER;
    }
    if (property == "ladder") {
        return dm.LADDER_TILE_NUMBER;
    }
    return 0;
}

} // namespace

// AS: MapManager.unsetProperty
void MapManager::unsetProperty(double x, double y, std::string_view property)
{
    data::DataManager &dm = *gameObj->obf_AQ;
    const double special = propertyTile(dm, property);
    if (special == 0) {
        return;
    }
    auto &tiles = dm.currentScreenData->tileArray;
    if (x >= 0 && y >= 0 && x < static_cast<double>(tiles[0].size()) && y < static_cast<double>(tiles.size())) {
        auto &cell = tiles[static_cast<size_t>(y)][static_cast<size_t>(x)];
        auto it = std::find(cell.begin(), cell.end(), special);
        if (it != cell.end()) {
            cell.erase(it);
        }
    }
}

void MapManager::setProperty(double x, double y, std::string_view property)
{
    data::DataManager &dm = *gameObj->obf_AQ;
    double special = 0;
    if (property == "collision") {
        special = dm.COLLISION_TILE_NUMBER;
    } else if (property == "slope") {
        special = dm.SLOPE_TILE_NUMBER;
    } else if (property == "thrublock") {
        special = dm.THRUBLOCK_TILE_NUMBER;
    } else if (property == "grab") {
        special = dm.GRAB_TILE_NUMBER;
    } else if (property == "ladder") {
        special = dm.LADDER_TILE_NUMBER;
    } else {
        return;
    }
    auto &tiles = dm.currentScreenData->tileArray;
    if (x >= 0 && y >= 0 && x < static_cast<double>(tiles[0].size()) && y < static_cast<double>(tiles.size())) {
        auto &cell = tiles[static_cast<size_t>(y)][static_cast<size_t>(x)];
        if (std::find(cell.begin(), cell.end(), special) == cell.end()) {
            cell.push_back(special);
        }
    }
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
    return 0; // "door" is getTileDoor
}

std::vector<double> MapManager::getTileDoor(double x, double y) const
{
    const data::DataManager &dm = *gameObj->obf_AQ;
    if (!isWithinBounds(x, y)) {
        return {};
    }
    const double doorID = dm.currentScreenData->checkForDoorID(x, y);
    if (doorID > -1) {
        return dm.currentWorldData->getDoorArray(dm.currentScreen, doorID);
    }
    return {};
}

} // namespace papaGame::managers
