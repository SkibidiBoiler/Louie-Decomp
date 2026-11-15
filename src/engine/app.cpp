#include "engine/app.h"
#include "engine/paths.h"

namespace engine {

namespace {
// Never run more than this many logic frames per display frame (e.g. after a stall or a breakpoint).
constexpr uint64_t kMaxCatchUpFrames = 5;
} // namespace

App::~App()
{
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
    }
    if (window_) {
        SDL_DestroyWindow(window_);
    }
}

SDL_AppResult App::init(int argc, char *argv[])
{
    SDL_SetAppMetadata("Papa's Louie Recomp", "0.1.0", "io.github.skibidiboiler.papalouie");
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
#if defined(SDL_PLATFORM_ANDROID)
    flags |= SDL_WINDOW_FULLSCREEN;
#endif
    if (!SDL_CreateWindowAndRenderer("Papa's Louie Recomp", stage_.width * 2, stage_.height * 2, flags,
                                     &window_, &renderer_)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window/renderer creation failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetRenderVSync(renderer_, 1);
    SDL_SetRenderLogicalPresentation(renderer_, stage_.width, stage_.height, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    dataDir_ = resolveDataDir(argc, argv);
    hasData_ = directoryExists(dataDir_);
    SDL_Log("Renderer: %s", SDL_GetRendererName(renderer_));
    SDL_Log("Data dir: %s (%s)", dataDir_.c_str(), hasData_ ? "found" : "missing");

    stepNs_ = static_cast<uint64_t>(1e9 / stage_.frameRate);
    lastNs_ = SDL_GetTicksNS();
    return SDL_APP_CONTINUE;
}

SDL_AppResult App::event(const SDL_Event &ev)
{
    switch (ev.type) {
    case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;
    case SDL_EVENT_KEY_DOWN:
        if (ev.key.key == SDLK_ESCAPE || ev.key.key == SDLK_AC_BACK) {
            return SDL_APP_SUCCESS;
        }
        break;
    default:
        break;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult App::iterate()
{
    const uint64_t now = SDL_GetTicksNS();
    accumulatorNs_ += now - lastNs_;
    lastNs_ = now;
    if (accumulatorNs_ > kMaxCatchUpFrames * stepNs_) {
        accumulatorNs_ = kMaxCatchUpFrames * stepNs_;
    }
    while (accumulatorNs_ >= stepNs_) {
        tick();
        accumulatorNs_ -= stepNs_;
    }
    render(static_cast<double>(accumulatorNs_) / static_cast<double>(stepNs_));
    return SDL_APP_CONTINUE;
}

void App::tick()
{
    ++frame_;
    // SKELETON: probe crosses the stage once every 2 seconds of game time.
    probePrevX_ = probeX_;
    probeX_ += stage_.width / (2.0 * stage_.frameRate);
    if (probeX_ >= stage_.width) {
        probeX_ -= stage_.width;
        probePrevX_ = probeX_;
    }
}

void App::render(double alpha)
{
    // Green background when the data folder was found, red when it is missing.
    if (hasData_) {
        SDL_SetRenderDrawColor(renderer_, 24, 96, 48, 255);
    } else {
        SDL_SetRenderDrawColor(renderer_, 110, 28, 28, 255);
    }
    SDL_RenderClear(renderer_);

    const double x = probePrevX_ + (probeX_ - probePrevX_) * alpha;
    const SDL_FRect probe{static_cast<float>(x), static_cast<float>(stage_.height) * 0.5f - 16.0f, 32.0f, 32.0f};
    SDL_SetRenderDrawColor(renderer_, 255, 220, 120, 255);
    SDL_RenderFillRect(renderer_, &probe);

    SDL_RenderPresent(renderer_);
}

} // namespace engine
