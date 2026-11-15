#pragma once
// AS: papaGame.data.WorldData — a world (level): its rooms, the doors between them and the map screen.
#include <string>
#include <vector>

#include "engine/xml.h"
#include "game/as3.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::data {

class WorldData {
public:
    WorldData(obf_9G::obf_4R *gameObj, const engine::XmlNode &xml);

    void populateData(const engine::XmlNode &xml);
    std::vector<double> getWorldRoomIDs() const;

    obf_9G::obf_4R *gameObj = nullptr;
    double worldID = as3::NaN;
    std::vector<std::vector<double>> worldDoorArray;
    std::vector<std::vector<double>> worldEventArray;
    std::vector<std::vector<as3::Value>> worldStateArray; // first 5 fields Number, the rest String
    double worldStartRoom = as3::NaN;
    std::vector<std::string> roomNameArray;
    std::vector<std::vector<double>> worldRoomArray;
    std::string worldTitle;
    double tileset = as3::NaN; // QUIRK: never assigned by populateData in the original
    std::vector<std::vector<double>> mapArray;
    std::vector<double> mapProgressArray;
};

} // namespace papaGame::data
