#pragma once
#include <SDL3/SDL.h>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "engine/movieclip.h"
#include "engine/settings.h"
#include "engine/sheets.h"
#include "engine/stage.h"
#include "engine/symbols.h"
#include "engine/textures.h"
#include "game/obf_9G/obf_4R.h"

namespace engine {

// Owns the window and renderer and drives the fixed-timestep loop.
// Game logic advances exactly once per Flash frame (Stage::frameRate), whatever the display refresh rate;
// rendering runs at Settings::fpsCap and interpolates between logic steps when enabled.
class App {
public:
    ~App();

    SDL_AppResult init(int argc, char *argv[]);
    SDL_AppResult event(const SDL_Event &ev);
    SDL_AppResult iterate();

private:
    void tick();
    void render(float alpha);
    void renderOverlay();
    bool captureFrame();

    // DEV: symbol viewer and blit demo, used until the first game screen is ported.
    void loadGame(const std::string &game);
    void showSymbol(int index);
    void renderBlitDemo(float alpha);
    void nextMode();
    void switchRoom(int delta);
    void tickLevel();

    SDL_Window *window_ = nullptr;
    SDL_Renderer *renderer_ = nullptr;
    Stage stage_;
    Settings settings_;
    std::string dataDir_;
    bool hasData_ = false;

    uint64_t stepNs_ = 0;
    uint64_t lastNs_ = 0;
    uint64_t accumulatorNs_ = 0;
    uint64_t frame_ = 0;

    uint64_t startNs_ = 0;
    std::string capturePath_; // DEV: --capture <png>
    bool quitRequested_ = false;
    uint64_t lastRenderNs_ = 0;
    uint64_t fpsWindowStartNs_ = 0;
    int fpsFrames_ = 0;
    float measuredFps_ = 0.0f;
    float displayHz_ = 0.0f;

    SymbolLibrary symbols_;
    ShapeTextures textures_;
    std::vector<std::pair<std::string, uint16_t>> viewerList_;
    int viewerIndex_ = 0;
    std::unique_ptr<MovieClip> viewerClip_;
    Affine viewerFit_;
    SheetLibrary sheets_;

    // DEV: what the app shows until the menus are ported. Level = PL2 level 0 with a free camera.
    enum class Mode { Level, BlitDemo, Viewer };
    Mode mode_ = Mode::Level;
    std::unique_ptr<obf_9G::obf_4R> game_;
    float dragX_ = 0.0f, dragY_ = 0.0f; // pending camera pan from mouse/touch drags, applied in tick
    bool dragging_ = false;

    // DEV: Enemy35 walking back and forth in the blit demo.
    double sargeX_ = 200.0;
    double sargePrevX_ = 200.0;
    int sargeDir_ = 1;
    int sargeCycle_ = 0;
};

} // namespace engine
