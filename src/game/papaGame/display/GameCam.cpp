#include "game/papaGame/display/GameCam.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::display {

// AS: GameCam.GameCam
GameCam::GameCam(obf_9G::obf_4R *gameObj_) : gameObj(gameObj_)
{
    gameObj->obf_71->getCamera(this);
}

// AS: GameCam.setCameraBounds
void GameCam::setCameraBounds()
{
    obf_9G::obf_4R *game = gameObj;
    const data::DataManager *manager = game->obf_AQ.get();
    GameDisplay *display = game->obf_71.get();
    const auto &tiles = manager->currentScreenData->tileArray;
    const double levelW = static_cast<double>(tiles[0].size()) * display->tileWidth;
    const double levelH = static_cast<double>(tiles.size()) * display->tileWidth;
    min_x = 0 + (display->centerXtile - 1) * display->tileWidth;
    min_y = 0 + (display->centerYtile - 1) * display->tileWidth;
    max_x = levelW - display->screen_Xoffset;
    max_y = levelH - display->screen_Yoffset - display->tileWidth / 2;
    screenbounds_offsetX = 0;
    screenbounds_offsetY = 0;
    screenbounds_minx = 0 + screenbounds_offsetX;
    screenbounds_miny = 0 + screenbounds_offsetY;
    // QUIRK: §_-5h§ is a plain MovieClip, which has no stageWidth/stageHeight: both reads are undefined, so
    // these are NaN and the later "> screenbounds_max" clamps never fire.
    screenbounds_maxx = as3::NaN - screenbounds_offsetX;
    screenbounds_maxy = as3::NaN - screenbounds_offsetY;
    x = 240;
    y = 672;
    targetX = 240;
    targetY = 672;
    wasLockedCamera = false;
    savePrevious();
    display->defineScreenBounds();
}

} // namespace papaGame::display
