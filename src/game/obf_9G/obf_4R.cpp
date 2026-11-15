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
    obf_71 = std::make_unique<papaGame::display::GameDisplay>(this);
    obf_3Y = std::make_unique<papaGame::display::GameCam>(this);
    playerObj = std::make_unique<papaGame::models::characters::CustomerChar>(this);
}

bool obf_4R::loadLevelData(const std::string &gameDir)
{
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
    // SKELETON: HUD, challenges and §_-52§ (start level: adjustCamera, music, transition) come later.
}

// AS: §_-4R§.§_-4X§
void obf_4R::obf_4X(double whichRoom)
{
    // SKELETON: saving enemies/objects/items, clearing bullets/effects, the player and the door position are
    // not ported yet; this is the room switch itself.
    obf_AQ->setActiveRoom(whichRoom);
    if (obf_AQ->currentScreenData == nullptr) {
        return;
    }
    obf_71->setupPreBlit();
    obf_71->currentXcoord = 0;
    obf_71->currentYcoord = 0;
    obf_3Y->setCameraBounds();
}

} // namespace obf_9G
