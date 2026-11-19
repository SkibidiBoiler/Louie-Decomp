#pragma once
// AS: papaGame.data.DataManager — level XML, the current world and its rooms.
// Ported so far: level loading and room setup (setupLevelScreens, setActiveRoom, clearLevelScreens).
#include <memory>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>

#include "engine/json.h"
#include "engine/xml.h"
#include "game/papaGame/data/LevelData.h"
#include "game/papaGame/data/ScreenData.h"
#include "game/papaGame/data/WorldData.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::data {

class DataManager {
public:
    explicit DataManager(obf_9G::obf_4R *gameObj);

    // Port: replaces prepareLevelData/generateLevelData (the XML is decoded offline into levels.xml).
    bool prepareLevelData(const std::string &levelsXmlPath);
    void levelDataLoaded(bool loaded);

    // Port: loads the constant tables (datamanager.json, read from the SWF by the importer).
    bool loadTables(const std::string &path);
    double getPropValue(std::string_view property, const ScreenData::TileCell &cell) const;
    void setupCollisionArrays();

    void setupLevelScreens(double whichLevel);
    std::string getWorldTitle(double whichWorld) const;
    void setActiveRoom(double index);
    void initializeScreenObjects(double whichScreen);
    void saveCheckpoint(double tileX = -1, double tileY = -1);
    void clearCheckpoint();
    double getOutfitPrice(double customer, double outfit) const; // AS: DataManager.getOutfitPrice
    void clearLevelScreens();
    bool isOnTrain() const;
    bool isAnimatedBackground() const;

    obf_9G::obf_4R *gameObj = nullptr;
    LevelData levelDataClass;
    std::vector<const engine::XmlNode *> masterLevelData;
    std::vector<const engine::XmlNode *> worldData;
    double totalLevels = 0;
    double totalWorlds = 0;
    std::vector<std::unique_ptr<ScreenData>> currentScreens;
    ScreenData *currentScreenData = nullptr;
    std::unique_ptr<WorldData> currentWorldData;
    double currentLevel = 1;
    double currentScreen = 0;
    double currentTileset = 1;

    // Constant tables (see loadTables).
    using Grid = std::vector<std::vector<double>>;
    Grid outfitPrices;                     // from datamanager.json
    std::vector<double> coinsToUnlockWorld; // from datamanager.json
    Grid standardCollisionArray, standardThruArray, standardGrabArray;
    const Grid *collisionArray = nullptr;
    const Grid *thruArray = nullptr;
    const Grid *grabArray = nullptr;
    double COLLISION_TILE_NUMBER = 0, SLOPE_TILE_NUMBER = 0, THRUBLOCK_TILE_NUMBER = 0, GRAB_TILE_NUMBER = 0;
    double LADDER_TILE_NUMBER = 0;

private:
    const engine::XmlNode *findWorld(double tileset) const;
    bool inList(const char *list, double tile) const;
    engine::JsonValue tables_;
    std::unordered_map<std::string, std::unordered_set<double>> lists_; // tile lists for indexOf
};

} // namespace papaGame::data
