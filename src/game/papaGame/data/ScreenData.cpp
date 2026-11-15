#include "game/papaGame/data/ScreenData.h"

namespace papaGame::data {

namespace {

std::vector<std::vector<double>> splitNumberGroups(const std::string &field)
{
    // ")" separates groups, "-" separates numbers inside a group.
    std::vector<std::vector<double>> out;
    for (const std::string &group : as3::split(field, ')')) {
        std::vector<double> numbers;
        for (const std::string &n : as3::split(group, '-')) {
            numbers.push_back(as3::Number(n));
        }
        out.push_back(std::move(numbers));
    }
    return out;
}

std::vector<std::vector<as3::Value>> splitMixedRows(const std::string &text, size_t numericFields)
{
    std::vector<std::vector<as3::Value>> rows;
    for (const std::string &row : as3::split(text, '|')) {
        std::vector<as3::Value> values;
        size_t i = 0;
        for (const std::string &v : as3::split(row, ',')) {
            values.push_back(i < numericFields ? as3::Value::num(as3::Number(v)) : as3::Value::str(v));
            ++i;
        }
        rows.push_back(std::move(values));
    }
    return rows;
}

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

// AS: ScreenData.ScreenData
ScreenData::ScreenData(obf_9G::obf_4R *gameObj_, const engine::XmlNode *xml, double whichTileset_, double roomIndex_)
    : gameObj(gameObj_), whichTileset(whichTileset_), objectsInitialized(false), roomIndex(roomIndex_)
{
    if (xml) {
        populateData(*xml);
    }
}

// AS: ScreenData.populateData
void ScreenData::populateData(const engine::XmlNode &xml)
{
    tileArray.clear();
    for (const std::string &row : as3::split(xml.childText("tileArray"), '|')) {
        std::vector<TileCell> cells;
        for (const std::string &cell : as3::split(row, ',')) {
            TileCell layers;
            for (const std::string &layer : as3::split(cell, '#')) {
                layers.push_back(as3::Number(layer)); // QUIRK: an empty cell becomes [0] and draws tile 0
            }
            cells.push_back(std::move(layers));
        }
        tileArray.push_back(std::move(cells));
    }
    itemArray = splitNumberRows(xml.childText("itemArray"));
    enemyArray = splitMixedRows(xml.childText("enemyArray"), 4);
    objectArray = splitMixedRows(xml.childText("objectArray"), 4);
    doorArray = splitNumberRows(xml.childText("doorArray"));

    startPoint.clear();
    for (const std::string &v : as3::split(xml.childText("startPoint"), ',')) {
        startPoint.push_back(as3::Number(v));
    }

    eventArray.clear();
    for (const std::string &entry : as3::split(xml.childText("eventArray"), '|')) {
        const std::vector<std::string> fields = as3::split(entry, ',');
        Event ev;
        ev.fieldCount = fields.size();
        if (fields.size() > 0) {
            ev.field0 = splitNumberGroups(fields[0]);
        }
        if (fields.size() > 1) {
            ev.field1 = splitNumberGroups(fields[1]);
        }
        if (fields.size() > 2) {
            ev.field2 = as3::Number(fields[2]);
        }
        if (fields.size() > 3) {
            ev.field3 = as3::Number(fields[3]);
        }
        if (fields.size() > 4) {
            ev.field4 = fields[4];
        }
        eventArray.push_back(std::move(ev));
    }

    if (xml.childText("itemArray").empty()) {
        itemArray.clear();
    }
    if (xml.childText("enemyArray").empty()) {
        enemyArray.clear();
    }
    if (xml.childText("objectArray").empty()) {
        objectArray.clear();
    }
    // (the original also clears an npcArray here; that field does not exist in this game)
    if (xml.childText("eventArray").empty()) {
        eventArray.clear();
    }
    if (xml.childText("doorArray").empty()) {
        doorArray.clear();
    }
    loadedRoom = true;
    roomID = as3::Number(xml.childText("roomID"));
}

// AS: ScreenData.checkForDoorID — the door whose tile rectangle [x, y, w, h, ...] holds (x, y), or -1.
double ScreenData::checkForDoorID(double x, double y) const
{
    double found = -1;
    for (size_t i = 0; i < doorArray.size(); ++i) {
        const auto &d = doorArray[i];
        if (d.size() >= 4 && x >= d[0] && x < d[0] + d[2] && y >= d[1] && y < d[1] + d[3]) {
            found = static_cast<double>(i);
            break;
        }
    }
    return found;
}

} // namespace papaGame::data
