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
    bool loadedRoom = false;
    double roomIndex = -1;
};

} // namespace papaGame::data
