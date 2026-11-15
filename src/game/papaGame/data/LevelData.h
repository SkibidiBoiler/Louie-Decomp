#pragma once
// AS: papaGame.data.LevelData — the level XML. The original decodes it from the pixels of a bitmap at load
// time (prepareXML/prepareXMLcycle); the port decodes it offline (tools/swf2assets.py -> levels.xml).
#include <memory>
#include <string>
#include <vector>

#include "engine/xml.h"

namespace papaGame::data {

class LevelData {
public:
    // Port: replaces prepareXML; reads <game dir>/levels.xml.
    bool load(const std::string &path);

    std::vector<const engine::XmlNode *> getLevelXMLList() const;
    std::vector<const engine::XmlNode *> getWorldXMLList() const;

private:
    std::unique_ptr<engine::XmlNode> xml_;
};

} // namespace papaGame::data
