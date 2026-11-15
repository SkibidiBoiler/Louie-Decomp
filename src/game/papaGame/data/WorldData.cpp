#include "game/papaGame/data/WorldData.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::data {

namespace {

std::vector<std::vector<double>> splitNumberRows(const std::string &text)
{
    std::vector<std::vector<double>> rows;
    for (const std::string &row : as3::split(text, '|')) {
        std::vector<double> values;
        for (const std::string &v : as3::split(row, ',')) {
            values.push_back(as3::Number(v));
        }
        rows.push_back(std::move(values));
    }
    return rows;
}

} // namespace

// AS: WorldData.WorldData
WorldData::WorldData(obf_9G::obf_4R *gameObj_, const engine::XmlNode &xml) : gameObj(gameObj_)
{
    populateData(xml);
}

// AS: WorldData.populateData
void WorldData::populateData(const engine::XmlNode &xml)
{
    worldID = as3::Number(xml.childText("worldID"));
    worldTitle = xml.childText("worldTitle");
    if (!xml.childText("startRoom").empty()) {
        worldStartRoom = as3::Number(xml.childText("startRoom")) - 1;
    }
    mapArray = splitNumberRows(xml.childText("mapArray"));

    worldRoomArray = splitNumberRows(xml.childText("roomArray"));
    mapProgressArray.assign(worldRoomArray.size(), 0.0);
    roomNameArray = as3::split(xml.childText("nameArray"), ',');

    worldDoorArray = splitNumberRows(xml.childText("doorArray"));
    if (xml.childText("doorArray").empty()) {
        worldDoorArray.clear();
    }

    worldStateArray.clear();
    for (const std::string &row : as3::split(xml.childText("stateArray"), '|')) {
        std::vector<as3::Value> values;
        size_t i = 0;
        for (const std::string &v : as3::split(row, ',')) {
            values.push_back(i >= 5 ? as3::Value::str(v) : as3::Value::num(as3::Number(v)));
            ++i;
        }
        worldStateArray.push_back(std::move(values));
    }
    if (xml.childText("stateArray").empty()) {
        worldStateArray.clear();
    }

    worldEventArray = splitNumberRows(xml.childText("eventArray"));
    // QUIRK: the original tests the non-existent "worldEventArray" tag, which always reads as "", so the
    // world event array always ends up empty.
    worldEventArray.clear();
}

// AS: WorldData.getWorldRoomIDs
std::vector<double> WorldData::getWorldRoomIDs() const
{
    std::vector<double> ids;
    for (const auto &room : worldRoomArray) {
        ids.push_back(room.size() > 6 ? room[6] : as3::NaN);
    }
    return ids;
}

// AS: WorldData.getDoorArray — [target room, target door, offsetX, offsetY, dirX, dirY]. worldDoorArray rows
// link two doors: [room A (1-based), door A, room B, door B].
std::vector<double> WorldData::getDoorArray(double screen, double doorID) const
{
    const DataManager &dm = *gameObj->obf_AQ;
    const auto &door = dm.currentScreens[static_cast<size_t>(screen)]->doorArray[static_cast<size_t>(doorID)];
    auto at = [&](size_t i) { return i < door.size() ? door[i] : as3::NaN; };
    for (const auto &link : worldDoorArray) {
        if (link.size() < 4) {
            continue;
        }
        if (link[0] == screen + 1 && link[1] == doorID) {
            return {link[2], link[3], at(2), at(3), at(4), at(5)};
        }
        if (link[2] == screen + 1 && link[3] == doorID) {
            return {link[0], link[1], at(2), at(3), at(4), at(5)};
        }
    }
    return {};
}

} // namespace papaGame::data
