#pragma once
// AS: papaGame.data.DataManager — level XML, the current world and its rooms.
// Ported so far: level loading and room setup (setupLevelScreens, setActiveRoom, clearLevelScreens).
#include <memory>
#include <string>
#include <vector>

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

    void setupLevelScreens(double whichLevel);
    std::string getWorldTitle(double whichWorld) const;
    void setActiveRoom(double index);
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

private:
    const engine::XmlNode *findWorld(double tileset) const;
};

} // namespace papaGame::data
