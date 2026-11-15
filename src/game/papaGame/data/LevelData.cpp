#include "game/papaGame/data/LevelData.h"

#include <SDL3/SDL.h>

namespace papaGame::data {

bool LevelData::load(const std::string &path)
{
    size_t size = 0;
    void *raw = SDL_LoadFile(path.c_str(), &size);
    if (!raw) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot read %s: %s", path.c_str(), SDL_GetError());
        return false;
    }
    xml_ = engine::parseXml(std::string_view(static_cast<const char *>(raw), size));
    SDL_free(raw);
    return xml_ != nullptr;
}

// AS: LevelData.getLevelXMLList
std::vector<const engine::XmlNode *> LevelData::getLevelXMLList() const
{
    const engine::XmlNode *levels = xml_ ? xml_->child("levels") : nullptr;
    return levels ? levels->childrenNamed("level") : std::vector<const engine::XmlNode *>{};
}

// AS: LevelData.getWorldXMLList
std::vector<const engine::XmlNode *> LevelData::getWorldXMLList() const
{
    const engine::XmlNode *worlds = xml_ ? xml_->child("worlds") : nullptr;
    return worlds ? worlds->childrenNamed("world") : std::vector<const engine::XmlNode *>{};
}

} // namespace papaGame::data
