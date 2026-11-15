#pragma once
#include <SDL3/SDL.h>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "engine/movieclip.h"
#include "engine/stage.h"
#include "engine/symbols.h"
#include "engine/textures.h"

namespace engine {

// Owns the window and renderer and drives the fixed-timestep loop.
// Game logic advances exactly once per Flash frame (Stage::frameRate), whatever the display refresh rate.
class App {
public:
    ~App();

    SDL_AppResult init(int argc, char *argv[]);
    SDL_AppResult event(const SDL_Event &ev);
    SDL_AppResult iterate();

private:
    void tick();
    void render(double alpha);

    // DEV: symbol viewer, used until the first game screen is ported.
    void loadGame(const std::string &game);
    void showSymbol(int index);

    SDL_Window *window_ = nullptr;
    SDL_Renderer *renderer_ = nullptr;
    Stage stage_;
    std::string dataDir_;
    bool hasData_ = false;

    uint64_t stepNs_ = 0;
    uint64_t lastNs_ = 0;
    uint64_t accumulatorNs_ = 0;
    uint64_t frame_ = 0;

    SymbolLibrary symbols_;
    ShapeTextures textures_;
    std::vector<std::pair<std::string, uint16_t>> viewerList_;
    int viewerIndex_ = 0;
    std::unique_ptr<MovieClip> viewerClip_;
    Affine viewerFit_;
};

} // namespace engine
