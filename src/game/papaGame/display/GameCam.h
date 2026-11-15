#pragma once
// AS: papaGame.display.GameCam — the camera (centre of the screen, in level pixels).
// Ported so far: fields and setCameraBounds. adjustCamera/panCamera/jiggle come with the player.
#include "game/as3.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::display {

class GameCam {
public:
    explicit GameCam(obf_9G::obf_4R *gameObj);

    void setCameraBounds();

    // Port: remember the position before a logic step so rendering can interpolate between steps.
    void savePrevious() { prevX = x; prevY = y; }

    obf_9G::obf_4R *gameObj = nullptr;
    double x = 240;
    double y = 672;
    double targetX = 240;
    double targetY = 672;
    double min_x = as3::NaN;
    double max_x = as3::NaN;
    double min_y = as3::NaN;
    double max_y = as3::NaN;
    double screenbounds_offsetX = as3::NaN;
    double screenbounds_offsetY = as3::NaN;
    double screenbounds_minx = as3::NaN;
    double screenbounds_maxx = as3::NaN;
    double screenbounds_miny = as3::NaN;
    double screenbounds_maxy = as3::NaN;
    bool lockedCamera = false;
    bool wasLockedCamera = false;

    double prevX = 240; // port
    double prevY = 672; // port
};

} // namespace papaGame::display
