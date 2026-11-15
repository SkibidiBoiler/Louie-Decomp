#pragma once
#include <SDL3/SDL.h>
#include <cstdint>
#include <string>

#include "engine/stage.h"

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

    SDL_Window *window_ = nullptr;
    SDL_Renderer *renderer_ = nullptr;
    Stage stage_;
    std::string dataDir_;
    bool hasData_ = false;

    uint64_t stepNs_ = 0;
    uint64_t lastNs_ = 0;
    uint64_t accumulatorNs_ = 0;
    uint64_t frame_ = 0;

    // SKELETON: test pattern to check the timestep; remove when the first game scene is ported.
    double probeX_ = 0.0;
    double probePrevX_ = 0.0;
};

} // namespace engine
