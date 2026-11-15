#include "game/obf_9G/obf_4R.h"

namespace obf_9G {

obf_4R::obf_4R(SDL_Renderer *renderer_, engine::SheetLibrary *sheets, const engine::SymbolLibrary *symbols_,
               engine::ShapeTextures *textures_)
    : renderer(renderer_), symbols(symbols_), textures(textures_)
{
    obf_51 = std::make_unique<papaGame::data::UserData>(this);
    obf_3A = std::make_unique<papaGame::data::CustomerData>(this);
    // Same order as the original (§_-AP§ creates the DataManager, §_-9k§ the rest).
    obf_AQ = std::make_unique<papaGame::data::DataManager>(this);
    bitmapManager = std::make_unique<papaGame::managers::BitmapManager>(this, sheets);
    obf_4i = std::make_unique<papaGame::managers::MapManager>(this);
    obf_71 = std::make_unique<papaGame::display::GameDisplay>(this);
    obf_3Y = std::make_unique<papaGame::display::GameCam>(this);
    playerObj = std::make_unique<papaGame::models::characters::CustomerChar>(this);
    obf_8o = std::make_unique<papaGame::events::GameControls>(this);
    obf_E = std::make_unique<papaGame::managers::EnemyManager>(this);
    obf_7u = std::make_unique<papaGame::managers::ObjectManager>(this);
    obf_3Z = std::make_unique<papaGame::managers::ItemManager>(this);
}

bool obf_4R::loadLevelData(const std::string &gameDir)
{
    obf_AQ->loadTables(gameDir + "/datamanager.json");
    playerObj->loadTables(gameDir + "/customerchar.json");
    obf_E->loadTables(gameDir + "/enemies.json");
    obf_7u->loadTables(gameDir + "/objects.json");
    obf_3Z->loadTables(gameDir + "/items.json");
    return obf_AQ->prepareLevelData(gameDir + "/levels.xml");
}

// AS: §_-4R§.§_-6V§
void obf_4R::obf_6V(double whichLevel)
{
    obf_AQ->currentLevel = whichLevel;
    // SKELETON: lives, tallies and challenges (§_-51§, §_-94§) are not ported yet.
    bitmapManager->whichTileset = -1;
    obf_AQ->setupLevelScreens(whichLevel);
    if (obf_AQ->currentScreenData == nullptr) {
        return; // port: no level data
    }
    obf_71->setupPreBlit();
    obf_71->currentXcoord = 0;
    obf_71->currentYcoord = 0;
    obf_3Y->setCameraBounds();
    if (!playerObj->blitter) {
        // SKELETON: the original blits the player sprite on the screens before the level starts.
        playerObj->blitter = std::make_unique<papaGame::managers::CharacterBlitter>(this);
        playerObj->blitter->createPlayerSprite();
    }
    const auto &start = obf_AQ->currentScreenData->startPoint;
    if (start.size() >= 3) {
        playerObj->startupPlayer(start[1], start[2], true);
    }
    // SKELETON: HUD, challenges and the level intro; the original reaches §_-52§ after the intro transition.
    obf_52();
}

// AS: §_-4R§.§_-52§ (start level)
void obf_4R::obf_52()
{
    obf_3Y->adjustCamera(playerObj->x, playerObj->y, true, true);
    obf_71->currentXcoord = 0;
    obf_71->currentYcoord = 0;
    obf_3Y->savePrevious(); // port: no interpolation across the snap
    obf_AQ->initializeScreenObjects(0);
    // SKELETON: level music and the portal sound.
}

// AS: §_-4R§.§_-4X§ — enter room `roomID` through its door `doorID`.
// The door's offsets are passed along but not read: the player is placed by the door's direction.
void obf_4R::obf_4X(double roomID, double doorID, double /*xOffset*/, double /*yOffset*/, double dirX, double dirY)
{
    obf_E->saveEnemies();
    obf_7u->saveObjects();
    obf_3Z->saveItems();
    // SKELETON: clearing bullets/effects and extra sprites.
    obf_AQ->setActiveRoom(roomID);
    if (obf_AQ->currentScreenData == nullptr) {
        return;
    }
    obf_71->setupPreBlit();
    obf_71->currentXcoord = 0;
    obf_71->currentYcoord = 0;
    obf_3Y->setCameraBounds();
    // SKELETON: HUD updateDisplay / updateTimer.
    const auto &doors = obf_AQ->currentScreenData->doorArray;
    if (doorID >= 0 && doorID < static_cast<double>(doors.size()) && doors[static_cast<size_t>(doorID)].size() >= 4) {
        const auto &door = doors[static_cast<size_t>(doorID)];
        const double tileX = door[0] + dirX;
        const double tileY = door[1] + (door[3] - 1) + dirY;
        playerObj->startupPlayer(tileX, tileY, false, true);
    }
    obf_3Y->adjustCamera(playerObj->x, playerObj->y, true, true);
    obf_3Y->savePrevious(); // port: no interpolation across the room switch
    obf_71->currentXcoord = 0;
    obf_71->currentYcoord = 0;
    obf_AQ->initializeScreenObjects(0);
    // SKELETON: HUD.
    obf_8o->stopCycle = false;
    obf_71->startTransition("in");
}

} // namespace obf_9G
