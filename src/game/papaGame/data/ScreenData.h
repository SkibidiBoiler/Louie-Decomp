#pragma once
// AS: papaGame.data.ScreenData — one room of a world: tiles, items, enemies, objects, events, doors, start.
#include <string>
#include <vector>

#include "engine/xml.h"
#include "game/as3.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::data {

class ScreenData {
public:
    // A tile cell holds one or more tile indices into the tilesheet ("a#b" in the XML), drawn in order.
    using TileCell = std::vector<double>;

    // One eventArray entry: "a-b)c-d,e-f,n,n,str". Field meanings are not known yet.
    struct Event {
        size_t fieldCount = 0;
        std::vector<std::vector<double>> field0;
        std::vector<std::vector<double>> field1;
        double field2 = as3::NaN;
        double field3 = as3::NaN;
        std::string field4;
    };

    ScreenData(obf_9G::obf_4R *gameObj, const engine::XmlNode *xml = nullptr, double whichTileset = 0,
               double roomIndex = -1);

    void populateData(const engine::XmlNode &xml);
    double checkForDoorID(double x, double y) const;
    // AS: ScreenData.saveEnemyState — a killed enemy's row becomes [0], so it is not built again.
    // AS: ScreenData.saveObjectState
    void saveObjectState(double index, const std::vector<as3::Value> &row, bool force = false)
    {
        if (index >= 0 && (force || index < static_cast<double>(objectArray.size()))) {
            if (index >= static_cast<double>(objectArray.size())) {
                objectArray.resize(static_cast<size_t>(index) + 1);
            }
            objectArray[static_cast<size_t>(index)] = row;
        }
    }
    void saveEnemyState(double index, const std::vector<as3::Value> &row)
    {
        if (index >= 0 && index < static_cast<double>(enemyArray.size())) {
            enemyArray[static_cast<size_t>(index)] = row;
        }
    }

    obf_9G::obf_4R *gameObj = nullptr;
    std::vector<std::vector<TileCell>> tileArray;
    std::vector<Event> eventArray;
    std::vector<std::vector<double>> doorArray;
    std::vector<double> startPoint;
    std::vector<std::vector<as3::Value>> enemyArray;  // first 4 fields Number, the rest String
    std::vector<std::vector<as3::Value>> objectArray; // first 4 fields Number, the rest String
    std::vector<std::vector<double>> itemArray;
    double roomID = as3::NaN;
    double whichTileset = 0;
    bool objectsInitialized = false;
    double objectTotal = 0;
    bool loadedRoom = false;
    double roomIndex = -1;
};

} // namespace papaGame::data
