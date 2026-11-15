#pragma once
// AS: papaGame.display.GameDisplay — draws a frame of the game into screenBMP (700x416) with copyPixels.
// The port draws straight onto the current render target instead of a BitmapData.
// Ported so far: screen bounds, sky, backdrop and tiles (the first steps of redrawScreen).
#include <string>

#include "engine/sheets.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::display {

class GameCam;

class GameDisplay {
public:
    explicit GameDisplay(obf_9G::obf_4R *gameObj);
    ~GameDisplay();
    GameDisplay(const GameDisplay &) = delete;
    GameDisplay &operator=(const GameDisplay &) = delete;

    void getCamera(GameCam *cam);
    void setupPreBlit();
    double createBackground(double whichTileset);
    void defineScreenBounds();

    // Port: what redrawScreen does to the display state once per logic frame (the original runs it while
    // drawing, which is the same thing at the SWF frame rate).
    void advanceFrame();
    // AS: redrawScreen (so far: defineScreenBounds, reblitSky, reblitBackdrop, reblitTiles).
    // `alpha` interpolates the camera and animated offsets between the last two logic frames; with
    // `smooth` the camera is not snapped to whole stage pixels (port option, off = original look).
    void redrawScreen(float alpha, bool smooth);

    void reblitSky();
    void reblitBackdrop();
    void reblitTiles();

    obf_9G::obf_4R *gameObj = nullptr;
    GameCam *camObj = nullptr;
    double tileWidth = 32;
    double screenTileWidth = 23;
    double screenTileHeight = 13;
    std::string bgScrollType = "scroll";
    double bgScrollSpeedX = 3;
    double bgScrollSpeedY = 3;
    const engine::Sheet *tileSheet = nullptr;
    const engine::Sheet *bgSheet = nullptr;
    const engine::Sheet *mgSheet = nullptr;
    double tileSheetWidth = 62;
    double tileSheetHeight = 36;
    double currentLevel = 0;
    double centerXtile = 0;
    double centerYtile = 0;
    double screenPxWidth = 700;
    double screenPxHeight = 416;
    double screen_Xoffset = 350;
    double screen_Yoffset = 208;
    double levelPxWidth = 0;
    double levelPxHeight = 0;
    double levelTileWidth = 0;
    double levelTileHeight = 0;
    double currentXtileLeft = 0;
    double currentXtileRight = 0;
    double currentYtileUp = 0;
    double currentYtileDown = 0;
    double currentXcoord = 0;
    double currentYcoord = 0;
    double trainX = 0;
    double trainSpeed = 25;
    bool showLightning = false;
    double lightningTimer = 0;

private:
    void blit(const engine::Sheet &sheet, double sx, double sy, double sw, double sh, double dx, double dy) const;

    // Port: interpolation state for redrawScreen.
    double prevTrainX_ = 0;
    double drawCamX_ = 0, drawCamY_ = 0, drawTrainX_ = 0;
    bool smooth_ = false;
    // Port: the visible tiles, drawn texel-exact before being placed on screen (stands in for tileBMP).
    SDL_Texture *tileLayer_ = nullptr;
    float tileLayerScale_ = 0.0f;
};

} // namespace papaGame::display
