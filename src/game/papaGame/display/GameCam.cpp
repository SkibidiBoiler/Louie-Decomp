#include "game/papaGame/display/GameCam.h"

#include "game/obf_9G/obf_4R.h"

#include <cmath>

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

// AS: GameCam.adjustCamera
void GameCam::adjustCamera(double anchorX, double anchorY, bool anchorIsPlayerChar, bool snap)
{
    obf_9G::obf_4R *game = gameObj;
    const GameDisplay *display = game->obf_71.get();
    // isOnBarge() is always false in this game: the barge restore at the start and the barge bob at the end
    // never run and are left out. So are two mouse-look values (from mouseRatioToScreen) the original computes
    // and never reads.
    double mouseX = 0;
    double mouseY = 0;
    double offset = cameraOffset;
    if (offset < maxCameraOffset * -1) {
        offset = maxCameraOffset * -1;
    } else if (offset > maxCameraOffset) {
        offset = maxCameraOffset;
    }
    if (mouseX < screenbounds_minx) {
        mouseX = screenbounds_minx;
    }
    if (mouseX > screenbounds_maxx) {
        mouseX = screenbounds_maxx;
    }
    if (mouseY < screenbounds_miny) {
        mouseY = screenbounds_miny;
    }
    if (mouseY > screenbounds_maxy) {
        mouseY = screenbounds_maxy;
    }
    mouseX -= screenbounds_offsetX;
    mouseY -= screenbounds_offsetY;
    mouseRealX = mouseX;
    mouseRealY = mouseY;
    if (anchorIsPlayerChar) {
        targetX = anchorX;
        targetY = anchorY + offset - 32;
    } else {
        targetX = anchorX;
        targetY = anchorY - 32;
    }
    if (lockedCamera) {
        targetX = lockTargetX;
        targetY = lockTargetY;
    } else if (forcedScrollingCamera) {
        if (forcedScrollingDirX != 0) {
            targetX = x + forcedScrollingDirX * forcedScrollingSpeed;
        }
        if (forcedScrollingDirY != 0) {
            targetY = y + forcedScrollingDirY * forcedScrollingSpeed;
        }
    }
    if (targetX > max_x) {
        targetX = max_x;
    }
    if (targetX < min_x) {
        targetX = min_x;
    }
    if (targetY > max_y) {
        targetY = max_y;
    }
    if (targetY < min_y) {
        targetY = min_y;
    }
    if (!snap) {
        const double distX = targetX - x;
        const double distY = targetY - y;
        double stepX = std::floor(distX / chasesteps);
        double stepY = std::floor(distY / chasesteps);
        if (anchorIsPlayerChar) {
            stepY = std::floor(distY / (chasesteps * 2));
        }
        if (wasLockedCamera && std::abs(distX) < 5 && std::abs(distY) < 5) {
            wasLockedCamera = false;
        }
        if (lockedCamera || wasLockedCamera) {
            stepX = std::floor(distX / lockchasesteps);
            stepY = std::floor(distY / lockchasesteps);
        }
        x += stepX;
        y += stepY;
        if (forcedScrollingCamera) {
            if (forcedScrollingDirX != 0) {
                x = targetX;
            }
            if (forcedScrollingDirY != 0) {
                y = targetY;
            }
        }
        if (lockedCamera && lockStoppingPlayer && std::abs(stepX) < 3 && std::abs(stepY) < 3) {
            lockStoppingPlayer = false;
            game->obf_8o->stopActionControls = false;
        }
    } else {
        x = targetX;
        y = targetY;
    }
    const double screenLeft = std::floor(x) - std::floor(display->screenTileWidth / 2) * display->tileWidth;
    const double screenTop = std::floor(y) - std::floor(display->screenTileHeight / 2) * display->tileWidth;
    mouseLookX = std::floor(screenLeft + mouseX);
    mouseLookY = std::min(anchorY, std::floor(screenTop + mouseY));
    mouseTargetX = std::floor(screenLeft + mouseX);
    mouseTargetY = std::floor(screenTop + mouseY);
    // SKELETON: jiggleCamera (screen shake) runs here when jiggle || isOnTrain().
    if (x > max_x) {
        x = max_x;
    }
    if (x < min_x) {
        x = min_x;
    }
    if (y > max_y) {
        y = max_y;
    }
    if (y < min_y) {
        y = min_y;
    }
    last_x = x;
    last_y = y;
}

} // namespace papaGame::display
