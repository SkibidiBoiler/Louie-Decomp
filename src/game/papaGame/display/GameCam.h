#pragma once
// AS: papaGame.display.GameCam — the camera (centre of the screen, in level pixels).
// Ported so far: fields, setCameraBounds, adjustCamera, the camera lock and the jiggle. Mouse panning and
// panCamera come later.
#include "game/as3.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::display {

class GameCam {
public:
    explicit GameCam(obf_9G::obf_4R *gameObj);

    void setCameraBounds();
    // The original takes the anchor object (`param1:*`) and tests `param1 is PlayerChar`; the port passes the
    // anchor's position and the result of that test.
    void adjustCamera(double anchorX, double anchorY, bool anchorIsPlayerChar, bool snap = false);

    void setCameraJiggle(double amount, double duration);
    void setCameraLock(double tileX, double tileY, bool stopPlayer = true);
    void unlockCamera();
    void jiggleCamera();
    // Port: remember the position before a logic step so rendering can interpolate between steps.
    void unsetCameraForceScrolling() { forcedScrollingCamera = false; } // AS: GameCam.unsetCameraForceScrolling

    void savePrevious() { prevX = x; prevY = y; }

    obf_9G::obf_4R *gameObj = nullptr;
    double x = 240;
    double y = 672;
    double targetX = 240;
    double targetY = 672;
    double mouseLookX = 240;
    double mouseLookY = 672;
    double mouseTargetX = as3::NaN;
    double mouseTargetY = as3::NaN;
    double mouseRealX = as3::NaN;
    double mouseRealY = as3::NaN;
    double chasesteps = 2;
    double lockchasesteps = 8;
    double min_x = as3::NaN;
    double max_x = as3::NaN;
    double min_y = as3::NaN;
    double max_y = as3::NaN;
    double last_x = 0;
    double last_y = 0;
    double mouseRatioToScreen = 4;
    double screenbounds_offsetX = as3::NaN;
    double screenbounds_offsetY = as3::NaN;
    double screenbounds_minx = as3::NaN;
    double screenbounds_maxx = as3::NaN;
    double screenbounds_miny = as3::NaN;
    double screenbounds_maxy = as3::NaN;
    bool lockedCamera = false;
    bool wasLockedCamera = false;
    bool lockStoppingPlayer = false;
    double lockTargetX = 0;
    double lockTargetY = 0;
    bool forcedScrollingCamera = false;
    double forcedScrollingDirX = 0;
    double forcedScrollingDirY = 0;
    double forcedScrollingSpeed = 1;
    bool jiggle = false;
    double jiggleamount = 0;
    double jiggleduration = 0;
    double currentjiggle = 0;
    double lastjigglex = 1;
    double lastjiggley = 1;
    double jiggleTrainAmount = 4;
    double jiggleTrainDelay = 0;
    double jiggleTrainDelayMax = 8;
    double jiggleTrainDelayShort = 8;
    double jiggleTrainDelayLong = 45;
    double cameraOffset = 0;
    double maxCameraOffset = 160;

    double prevX = 240; // port
    double prevY = 672; // port
};

} // namespace papaGame::display
