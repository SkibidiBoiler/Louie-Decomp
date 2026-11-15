#include "game/papaGame/data/DataManager.h"

#include <SDL3/SDL.h>

#include "game/as3.h"

namespace papaGame::data {

// AS: DataManager.DataManager
DataManager::DataManager(obf_9G::obf_4R *gameObj_) : gameObj(gameObj_)
{
}

bool DataManager::prepareLevelData(const std::string &levelsXmlPath)
{
    const bool ok = levelDataClass.load(levelsXmlPath);
    levelDataLoaded(ok);
    return ok;
}

// AS: DataManager.levelDataLoaded
void DataManager::levelDataLoaded(bool loaded)
{
    if (loaded) {
        masterLevelData = levelDataClass.getLevelXMLList();
        totalLevels = static_cast<double>(masterLevelData.size());
        worldData = levelDataClass.getWorldXMLList();
        totalWorlds = static_cast<double>(worldData.size());
    }
}

// worldData.(tileset == String(n))[0]
const engine::XmlNode *DataManager::findWorld(double tileset) const
{
    const std::string wanted = as3::String(tileset);
    for (const engine::XmlNode *world : worldData) {
        if (world->childText("tileset") == wanted) {
            return world;
        }
    }
    return nullptr;
}

// AS: DataManager.setupLevelScreens
void DataManager::setupLevelScreens(double whichLevel)
{
    SDL_Log(">>>>> Setup Level Screens: %s", as3::String(whichLevel).c_str());
    currentTileset = whichLevel;
    const engine::XmlNode *worldXML = findWorld(whichLevel);
    if (worldXML == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "WORLD DOESN'T EXIST YET! Fixer -- Default to World 2.");
        whichLevel = 1;
        worldXML = findWorld(whichLevel);
        if (worldXML == nullptr) {
            return; // port: no levels.xml loaded (the original would throw here)
        }
    }
    clearLevelScreens();
    const double whichTileset = whichLevel;
    currentLevel = whichLevel;
    currentWorldData = std::make_unique<WorldData>(gameObj, *worldXML);
    // SKELETON: the checkpoint branch (checkpointData) is not ported yet; this is the "Normal Loading" path.
    currentScreen = currentWorldData->worldStartRoom;
    const std::vector<double> roomArray = currentWorldData->getWorldRoomIDs();
    for (size_t i = 0; i < roomArray.size(); ++i) {
        const std::string id = as3::String(roomArray[i]);
        const engine::XmlNode *useXML = nullptr;
        for (const engine::XmlNode *level : masterLevelData) {
            if (level->childText("roomID") == id) {
                useXML = level;
                break;
            }
        }
        auto thisScreen = std::make_unique<ScreenData>(gameObj, useXML, whichTileset, static_cast<double>(i));
        if (static_cast<double>(i) == currentScreen) {
            currentScreenData = thisScreen.get();
        }
        currentScreens.push_back(std::move(thisScreen));
    }
    // SKELETON: setupCollisionArrays() comes with the collision code (milestone B).
}

// AS: DataManager.getWorldTitle
std::string DataManager::getWorldTitle(double whichWorld) const
{
    const engine::XmlNode *worldXML = findWorld(whichWorld);
    return worldXML ? worldXML->childText("worldTitle") : "Doesn't Exist Yet!";
}

// AS: DataManager.setActiveRoom
void DataManager::setActiveRoom(double index)
{
    if (currentScreenData) {
        currentScreenData->objectsInitialized = false;
        // clearBitmaps(): the port keeps no per-room tile bitmap (GameDisplay draws the tiles directly).
    }
    const auto i = static_cast<size_t>(index);
    currentScreenData = i < currentScreens.size() ? currentScreens[i].get() : nullptr;
    currentScreen = index;
}

// AS: DataManager.clearLevelScreens
void DataManager::clearLevelScreens()
{
    currentScreenData = nullptr;
    currentWorldData.reset();
    currentScreens.clear();
}

// AS: DataManager.isOnTrain
bool DataManager::isOnTrain() const
{
    return false;
}

// AS: DataManager.isAnimatedBackground
bool DataManager::isAnimatedBackground() const
{
    return currentLevel == 9;
}

} // namespace papaGame::data
