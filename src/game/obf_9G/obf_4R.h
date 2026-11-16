#pragma once
// AS: §_-9G§.§_-4R§ — the game object (obfuscated name; it holds every manager and runs the game).
// Ported so far: the managers needed to show a room, the level start (§_-6V§) and the room switch (§_-4X§).
#include <SDL3/SDL.h>
#include <memory>
#include <string>

#include "engine/audio.h"
#include "engine/sheets.h"
#include "engine/symbols.h"
#include "engine/text.h"
#include "engine/textures.h"
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/UserData.h"
#include "game/papaGame/data/DataManager.h"
#include "game/papaGame/display/GameCam.h"
#include "game/papaGame/events/EventManager.h"
#include "game/papaGame/events/GameControls.h"
#include "game/papaGame/display/GameDisplay.h"
#include "game/papaGame/managers/BitmapManager.h"
#include "game/papaGame/managers/ChallengeManager.h"
#include "game/papaGame/managers/EffectManager.h"
#include "game/papaGame/managers/EnemyManager.h"
#include "game/papaGame/managers/ItemManager.h"
#include "game/papaGame/managers/MapManager.h"
#include "game/papaGame/managers/ObjectManager.h"
#include "game/papaGame/managers/SoundManager.h"
#include "game/papaGame/models/GameHUD.h"
#include "game/papaGame/models/characters/CustomerChar.h"

namespace obf_9G {

class obf_4R {
public:
    // Port: the engine services the translated code draws with.
    obf_4R(SDL_Renderer *renderer, engine::SheetLibrary *sheets, const engine::SymbolLibrary *symbols,
           engine::ShapeTextures *textures, engine::TextLibrary *text, engine::Audio *audio);

    // Port: replaces the level-data loading screen; reads <gameDir>/levels.xml.
    bool loadLevelData(const std::string &gameDir);

    // AS: §_-4R§.§_-6V§ — start level `whichLevel` (so far: rooms, pre-blit and camera bounds).
    void obf_6V(double whichLevel);
    void obf_52();
    // AS: §_-4R§.§_-4X§ — the room-switch part of entering a door (no player or objects yet).
    void obf_4X(double roomID, double doorID, double xOffset, double yOffset, double dirX, double dirY);
    // AS: §_-4R§.finishLevel — "BEAT LEVEL". SKELETON: the level-complete screen; the next level starts instead.
    void finishLevel();
    // AS: §_-4R§.§_-2K§ — out of lives. SKELETON: the game over screen; the level starts again instead.
    void obf_2K();
    // AS: §_-4R§.§_-9k§ — a fresh player, HUD and managers for a level (the original builds them per level).
    void obf_9k();
    // Port: a level change asked for from inside the game loop (finishLevel / §_-2K§ run from GameDisplay) happens
    // at the start of the next logic frame, when no translated object is running.
    void runPending();

    SDL_Renderer *renderer = nullptr; // port
    const engine::SymbolLibrary *symbols = nullptr; // port
    engine::ShapeTextures *textures = nullptr; // port
    engine::TextLibrary *text = nullptr; // port
    std::unique_ptr<papaGame::data::UserData> obf_51;
    std::unique_ptr<papaGame::data::CustomerData> obf_3A;
    std::unique_ptr<papaGame::data::DataManager> obf_AQ;
    std::unique_ptr<papaGame::managers::BitmapManager> bitmapManager;
    std::unique_ptr<papaGame::managers::MapManager> obf_4i;
    std::unique_ptr<papaGame::display::GameDisplay> obf_71;
    std::unique_ptr<papaGame::display::GameCam> obf_3Y;
    std::unique_ptr<papaGame::models::characters::CustomerChar> playerObj;
    std::unique_ptr<papaGame::events::GameControls> obf_8o;
    std::unique_ptr<papaGame::managers::EnemyManager> obf_E;
    std::unique_ptr<papaGame::managers::ObjectManager> obf_7u;
    std::unique_ptr<papaGame::managers::ItemManager> obf_3Z;
    std::unique_ptr<papaGame::managers::EffectManager> obf_V;
    std::unique_ptr<papaGame::managers::ChallengeManager> obf_94;
    std::unique_ptr<papaGame::events::EventManager> obf_7t;
    std::unique_ptr<papaGame::managers::SoundManager> obf_7C;
    double obf_8W = 0; // AS: §_-8W§, the world (level) being played
    std::string gameDir;      // port: where loadLevelData read the tables
    double pendingLevel = -1; // port: see runPending
    std::unique_ptr<papaGame::models::GameHUD> obf_6Z; // created by loadLevelData, once the tables exist
};

} // namespace obf_9G
