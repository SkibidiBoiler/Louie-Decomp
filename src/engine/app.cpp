#include "engine/app.h"
#include "engine/paths.h"

#include <algorithm>

namespace engine {

namespace {
// Never run more than this many logic frames per display frame (e.g. after a stall or a breakpoint).
constexpr uint64_t kMaxCatchUpFrames = 5;
} // namespace

App::~App()
{
    viewerClip_.reset();
    textures_.clear(); // release textures before the renderer
    sheets_.clear();
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
    if (hasData_) {
        loadGame("pl2");
    }

    stepNs_ = static_cast<uint64_t>(1e9 / stage_.frameRate);
    lastNs_ = SDL_GetTicksNS();
    return SDL_APP_CONTINUE;
}

void App::loadGame(const std::string &game)
{
    const std::string dir = dataDir_ + "/" + game;
    if (!symbols_.load(dir + "/symbols.bin")) {
        return;
    }
    // Pick the shape scale closest to the display: the stage is drawn at (window height / stage height).
    int pw = 0, ph = 0;
    SDL_GetWindowSizeInPixels(window_, &pw, &ph);
    const float wanted = static_cast<float>(ph) / static_cast<float>(stage_.height);
    int scale = 1;
    for (int s : {2, 3, 4}) {
        if (wanted > static_cast<float>(s) - 0.75f && directoryExists(dir + "/shapes/" + std::to_string(s) + "x")) {
            scale = s;
        }
    }
    textures_.init(renderer_, dir, scale);
    sheets_.init(renderer_, dir, scale);
    SDL_Log("Shapes at %dx (display needs %.2fx)", scale, static_cast<double>(wanted));

    for (const auto &entry : symbols_.classes()) {
        if (symbols_.kind(entry.second) == CharKind::Sprite) {
            viewerList_.push_back(entry);
        }
    }
    std::sort(viewerList_.begin(), viewerList_.end());
    auto start = std::find_if(viewerList_.begin(), viewerList_.end(),
                              [](const auto &e) { return e.first == "enemysheet_sarge"; });
    showSymbol(start == viewerList_.end() ? 0 : static_cast<int>(start - viewerList_.begin()));
}

void App::showSymbol(int index)
{
    if (viewerList_.empty()) {
        return;
    }
    const int n = static_cast<int>(viewerList_.size());
    viewerIndex_ = ((index % n) + n) % n;
    const auto &entry = viewerList_[static_cast<size_t>(viewerIndex_)];
    const Timeline *tl = symbols_.timeline(entry.second);
    viewerClip_ = tl ? std::make_unique<MovieClip>(symbols_, *tl) : nullptr;

    // Fit the first frame into 90% of the stage.
    viewerFit_ = Affine::translate(static_cast<float>(stage_.width) * 0.5f, static_cast<float>(stage_.height) * 0.5f);
    float x0 = 1e30f, y0 = 1e30f, x1 = -1e30f, y1 = -1e30f;
    if (viewerClip_) {
        viewerClip_->accumulateBounds(Affine{}, x0, y0, x1, y1);
    }
    if (x1 > x0 && y1 > y0) {
        const float sw = static_cast<float>(stage_.width) * 0.9f, sh = static_cast<float>(stage_.height) * 0.9f;
        const float s = std::min({sw / (x1 - x0), sh / (y1 - y0), 4.0f});
        viewerFit_ = viewerFit_ * Affine::scale(s) * Affine::translate(-(x0 + x1) * 0.5f, -(y0 + y1) * 0.5f);
    }
    SDL_Log("Viewer %d/%d: %s (id %u, %u frames, bounds %.0f,%.0f..%.0f,%.0f)", viewerIndex_ + 1, n,
            entry.first.c_str(), entry.second, tl ? tl->frameCount : 0, static_cast<double>(x0),
            static_cast<double>(y0), static_cast<double>(x1), static_cast<double>(y1));
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
        if (ev.key.key == SDLK_TAB) {
            blitDemo_ = !blitDemo_;
        } else if (ev.key.key == SDLK_RIGHT || ev.key.key == SDLK_SPACE) {
            showSymbol(viewerIndex_ + 1);
        } else if (ev.key.key == SDLK_LEFT) {
            showSymbol(viewerIndex_ - 1);
        }
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP: { // touches arrive here too (SDL synthesises mouse events)
        int w = 0, h = 0;
        SDL_GetWindowSize(window_, &w, &h);
        if (ev.button.y < static_cast<float>(h) * 0.2f) { // top band: switch mode
            blitDemo_ = !blitDemo_;
        } else if (!blitDemo_) {
            showSymbol(viewerIndex_ + (ev.button.x >= static_cast<float>(w) * 0.5f ? 1 : -1));
        }
        break;
    }
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
    if (viewerClip_) {
        viewerClip_->advance();
    }
}

void App::render(double alpha)
{
    (void)alpha; // timeline animation is frame-stepped like Flash; interpolation comes with game objects
    if (!hasData_) {
        SDL_SetRenderDrawColor(renderer_, 110, 28, 28, 255); // red: data folder missing
    } else {
        SDL_SetRenderDrawColor(renderer_, 200, 200, 210, 255);
    }
    SDL_RenderClear(renderer_);

    if (blitDemo_ && hasData_) {
        renderBlitDemo();
    } else if (viewerClip_) {
        const RenderContext ctx{renderer_, &symbols_, &textures_};
        viewerClip_->render(ctx, viewerFit_, ColorTransform{});
    }
    SDL_RenderPresent(renderer_);
}

void App::renderBlitDemo()
{
    // Background sheet as the original's backdrop, then two tile rows from the composed tilesheet.
    if (const Sheet *bg = sheets_.get("tilesheet1_bg")) {
        sheets_.blit(*bg, 0, 0, static_cast<float>(stage_.width), static_cast<float>(stage_.height), 0, 0);
    }
    if (const Sheet *tiles = sheets_.composeTilesheet("tilesheet1")) {
        sheets_.blit(*tiles, 0, 0, static_cast<float>(stage_.width), 96, 0, static_cast<float>(stage_.height) - 96);
    }
    // Enemy35 ("enemysheet_sarge"): 192x160 cells, 11 per row, mirrored rows start at y = 1280.
    if (const Sheet *sarge = sheets_.get("enemysheet_sarge")) {
        const float w = 192, h = 160;
        const int perRow = 11;
        const int frame = static_cast<int>((frame_ / 2) % 88);
        const float sx = static_cast<float>(frame % perRow) * w;
        const float sy = static_cast<float>(frame / perRow) * h;
        const float y = static_cast<float>(stage_.height) - 96 - 135; // feet on the tiles (spriteTargetY = 135)
        sheets_.blit(*sarge, sx, sy, w, h, 120, y);
        sheets_.blit(*sarge, sx, sy + 1280, w, h, 390, y);
    }
}

} // namespace engine
