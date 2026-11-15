#pragma once
// AS: §_-9G§.§_-4R§ — the game object (obfuscated name; it holds every manager and runs the game).
// Ported so far: the managers needed to show a room, the level start (§_-6V§) and the room switch (§_-4X§).
#include <SDL3/SDL.h>
#include <memory>
#include <string>

#include "engine/sheets.h"
#include "engine/symbols.h"
#include "engine/textures.h"
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/UserData.h"
#include "game/papaGame/data/DataManager.h"
#include "game/papaGame/display/GameCam.h"
#include "game/papaGame/events/GameControls.h"
#include "game/papaGame/display/GameDisplay.h"
#include "game/papaGame/managers/BitmapManager.h"
#include "game/papaGame/managers/MapManager.h"
#include "game/papaGame/models/characters/CustomerChar.h"

namespace obf_9G {

class obf_4R {
public:
    // Port: the engine services the translated code draws with.
    obf_4R(SDL_Renderer *renderer, engine::SheetLibrary *sheets, const engine::SymbolLibrary *symbols,
           engine::ShapeTextures *textures);

    // Port: replaces the level-data loading screen; reads <gameDir>/levels.xml.
    bool loadLevelData(const std::string &gameDir);

    // AS: §_-4R§.§_-6V§ — start level `whichLevel` (so far: rooms, pre-blit and camera bounds).
    void obf_6V(double whichLevel);
    void obf_52();
    // AS: §_-4R§.§_-4X§ — the room-switch part of entering a door (no player or objects yet).
    void obf_4X(double whichRoom);

    SDL_Renderer *renderer = nullptr; // port
    const engine::SymbolLibrary *symbols = nullptr; // port
    engine::ShapeTextures *textures = nullptr; // port
    std::unique_ptr<papaGame::data::UserData> obf_51;
    std::unique_ptr<papaGame::data::CustomerData> obf_3A;
    std::unique_ptr<papaGame::data::DataManager> obf_AQ;
    std::unique_ptr<papaGame::managers::BitmapManager> bitmapManager;
    std::unique_ptr<papaGame::managers::MapManager> obf_4i;
    std::unique_ptr<papaGame::display::GameDisplay> obf_71;
    std::unique_ptr<papaGame::display::GameCam> obf_3Y;
    std::unique_ptr<papaGame::models::characters::CustomerChar> playerObj;
    std::unique_ptr<papaGame::events::GameControls> obf_8o;
};

} // namespace obf_9G
