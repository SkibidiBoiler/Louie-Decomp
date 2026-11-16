#include "game/obf_9G/obf_4R.h"

namespace obf_9G {

obf_4R::obf_4R(SDL_Renderer *renderer_, engine::SheetLibrary *sheets, const engine::SymbolLibrary *symbols_,
               engine::ShapeTextures *textures_, engine::TextLibrary *text_, engine::Audio *audio)
    : renderer(renderer_), symbols(symbols_), textures(textures_), text(text_)
{
    obf_7C = std::make_unique<papaGame::managers::SoundManager>(this, audio);
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
    obf_V = std::make_unique<papaGame::managers::EffectManager>(this);
    obf_94 = std::make_unique<papaGame::managers::ChallengeManager>(this);
    obf_7t = std::make_unique<papaGame::events::EventManager>(this);
}

bool obf_4R::loadLevelData(const std::string &gameDir_)
{
    gameDir = gameDir_;
    obf_AQ->loadTables(gameDir + "/datamanager.json");
    playerObj->loadTables(gameDir + "/customerchar.json");
    obf_E->loadTables(gameDir + "/enemies.json");
    obf_7u->loadTables(gameDir + "/objects.json");
    obf_3Z->loadTables(gameDir + "/items.json");
    obf_V->loadTables(gameDir + "/effects.json");
    obf_94->loadTables(gameDir + "/challenges.json");
    obf_51->loadTables(gameDir + "/userdata.json");
    obf_6Z = std::make_unique<papaGame::models::GameHUD>(this);
    return obf_AQ->prepareLevelData(gameDir + "/levels.xml");
}

// AS: §_-4R§.§_-6V§
void obf_4R::obf_6V(double whichLevel)
{
    obf_AQ->currentLevel = whichLevel;
    obf_8W = whichLevel;
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
    // SKELETON: challenges and the level intro; the original reaches §_-52§ after the intro transition.
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
    obf_7C->startLevelMusic();
    obf_7C->playSound("portalsound_spinfallout.wav");
    // SKELETON: playerObj.startTrainSound (train and river levels).
    obf_6Z->showHUD();
    // SKELETON: setupControls, setupCycleCode, handleCheckpointProgress.
    obf_6Z->updateDisplay();
    obf_6Z->updatePlayerHealth(playerObj->currentHealth);
}

// AS: §_-4R§.§_-4X§ — enter room `roomID` through its door `doorID`.
// The door's offsets are passed along but not read: the player is placed by the door's direction.
void obf_4R::obf_4X(double roomID, double doorID, double /*xOffset*/, double /*yOffset*/, double dirX, double dirY)
{
    obf_E->saveEnemies();
    obf_7u->saveObjects();
    obf_3Z->saveItems();
    // SKELETON: BulletManager.clearBullets.
    obf_V->clearEffects();
    // SKELETON: BitmapManager.clearExtraSprites.
    obf_AQ->setActiveRoom(roomID);
    if (obf_AQ->currentScreenData == nullptr) {
        return;
    }
    obf_71->setupPreBlit();
    obf_71->currentXcoord = 0;
    obf_71->currentYcoord = 0;
    obf_3Y->setCameraBounds();
    obf_6Z->updateDisplay();
    obf_6Z->updateTimer();
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
    obf_6Z->updateDisplay();
    obf_6Z->showHUD();
    obf_8o->stopCycle = false;
    obf_71->startTransition("in");
}

// AS: §_-4R§.finishLevel
void obf_4R::finishLevel()
{
    SDL_Log("BEAT LEVEL");
    obf_8o->stopCycle = true;
    obf_8o->stopControls = true;
    obf_AQ->clearCheckpoint();
    // SKELETON: the analytics call, saving, the level-complete screen with the tallies and challenges.
    pendingLevel = obf_AQ->currentLevel + 1;
}

// AS: §_-4R§.§_-2K§
void obf_4R::obf_2K()
{
    // SKELETON: the game over screen (continue from the checkpoint or quit).
    obf_51->resetLives();
    pendingLevel = obf_AQ->currentLevel;
}

// AS: §_-4R§.§_-9k§ (the managers' tables are read again: in the original they are class constants)
void obf_4R::obf_9k()
{
    obf_V = std::make_unique<papaGame::managers::EffectManager>(this);
    playerObj = std::make_unique<papaGame::models::characters::CustomerChar>(this);
    obf_4i = std::make_unique<papaGame::managers::MapManager>(this);
    obf_7u = std::make_unique<papaGame::managers::ObjectManager>(this);
    obf_E = std::make_unique<papaGame::managers::EnemyManager>(this);
    obf_3Z = std::make_unique<papaGame::managers::ItemManager>(this);
    obf_7t = std::make_unique<papaGame::events::EventManager>(this);
    obf_71 = std::make_unique<papaGame::display::GameDisplay>(this);
    obf_3Y = std::make_unique<papaGame::display::GameCam>(this);
    obf_8o = std::make_unique<papaGame::events::GameControls>(this);
    playerObj->loadTables(gameDir + "/customerchar.json");
    obf_E->loadTables(gameDir + "/enemies.json");
    obf_7u->loadTables(gameDir + "/objects.json");
    obf_3Z->loadTables(gameDir + "/items.json");
    obf_V->loadTables(gameDir + "/effects.json");
    obf_6Z = std::make_unique<papaGame::models::GameHUD>(this);
}

void obf_4R::runPending()
{
    if (pendingLevel >= 0) {
        const double level = pendingLevel;
        pendingLevel = -1;
        obf_9k();
        obf_6V(level);
    }
}

} // namespace obf_9G
