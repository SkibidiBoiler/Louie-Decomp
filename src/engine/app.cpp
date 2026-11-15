#include "engine/app.h"
#include "engine/paths.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

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
    settings_.load();

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
    if (const SDL_DisplayMode *mode = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(window_))) {
        displayHz_ = mode->refresh_rate;
    }

    dataDir_ = resolveDataDir(argc, argv);
    hasData_ = directoryExists(dataDir_);
    SDL_Log("Renderer: %s, display %.1f Hz, fps cap %d, interpolation %s", SDL_GetRendererName(renderer_),
            static_cast<double>(displayHz_), settings_.fpsCap, settings_.interpolation ? "on" : "off");
    SDL_Log("Data dir: %s (%s)", dataDir_.c_str(), hasData_ ? "found" : "missing");
    if (hasData_) {
        loadGame("pl2");
    }

    stepNs_ = static_cast<uint64_t>(1e9 / stage_.frameRate);
    lastNs_ = SDL_GetTicksNS();
    fpsWindowStartNs_ = lastNs_;
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
    SDL_Log("Viewer %d/%d: %s (id %u, %u frames)", viewerIndex_ + 1, n, entry.first.c_str(), entry.second,
            tl ? tl->frameCount : 0);
}

SDL_AppResult App::event(const SDL_Event &ev)
{
    const int originalRate = static_cast<int>(std::lround(stage_.frameRate));
    switch (ev.type) {
    case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;
    case SDL_EVENT_KEY_DOWN:
        switch (ev.key.key) {
        case SDLK_ESCAPE:
        case SDLK_AC_BACK:
            return SDL_APP_SUCCESS;
        case SDLK_TAB:
            blitDemo_ = !blitDemo_;
            break;
        case SDLK_F:
            settings_.nextFpsCap(originalRate);
            settings_.save();
            break;
        case SDLK_I:
            settings_.interpolation = !settings_.interpolation;
            settings_.save();
            break;
        case SDLK_RIGHT:
        case SDLK_SPACE:
            showSymbol(viewerIndex_ + 1);
            break;
        case SDLK_LEFT:
            showSymbol(viewerIndex_ - 1);
            break;
        default:
            break;
        }
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP: { // touches arrive here too (SDL synthesises mouse events)
        int w = 0, h = 0;
        SDL_GetWindowSize(window_, &w, &h);
        const float fx = ev.button.x / static_cast<float>(w), fy = ev.button.y / static_cast<float>(h);
        if (fy < 0.2f) { // top band: switch demo/viewer
            blitDemo_ = !blitDemo_;
        } else if (fy > 0.8f && fx < 0.33f) { // bottom-left: FPS cap
            settings_.nextFpsCap(originalRate);
            settings_.save();
        } else if (fy > 0.8f && fx > 0.67f) { // bottom-right: interpolation
            settings_.interpolation = !settings_.interpolation;
            settings_.save();
        } else if (!blitDemo_) {
            showSymbol(viewerIndex_ + (fx >= 0.5f ? 1 : -1));
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

    // Render-rate cap. Vsync already limits us to the display rate; this lowers it further when asked.
    if (settings_.fpsCap > 0) {
        const uint64_t interval = static_cast<uint64_t>(1e9 / settings_.fpsCap);
        const uint64_t since = now - lastRenderNs_;
        if (since + 500000 < interval) { // allow 0.5 ms of jitter so vsync-aligned frames are not skipped
            SDL_DelayNS(std::min<uint64_t>(interval - since, 1000000));
            return SDL_APP_CONTINUE;
        }
        lastRenderNs_ = since < 2 * interval ? lastRenderNs_ + interval : now;
    } else {
        lastRenderNs_ = now;
    }

    render(static_cast<float>(accumulatorNs_) / static_cast<float>(stepNs_));

    ++fpsFrames_;
    if (now - fpsWindowStartNs_ >= 1000000000ull) {
        measuredFps_ = static_cast<float>(fpsFrames_) * 1e9f / static_cast<float>(now - fpsWindowStartNs_);
        fpsFrames_ = 0;
        fpsWindowStartNs_ = now;
    }
    return SDL_APP_CONTINUE;
}

void App::tick()
{
    ++frame_;
    if (viewerClip_) {
        viewerClip_->advance();
    }
    // DEV: walk cycle frames 16..31, one cell per logic step, as Enemy35.walkCycleFrames.
    sargePrevX_ = sargeX_;
    sargeX_ += 3.0 * sargeDir_;
    if (sargeX_ > stage_.width - 100.0 || sargeX_ < 100.0) {
        sargeDir_ = -sargeDir_;
    }
    sargeCycle_ = (sargeCycle_ + 1) % 16;
}

void App::render(float alpha)
{
    if (!settings_.interpolation) {
        alpha = 1.0f; // original look: always the latest logic state
    }
    if (!hasData_) {
        SDL_SetRenderDrawColor(renderer_, 110, 28, 28, 255); // red: data folder missing
    } else {
        SDL_SetRenderDrawColor(renderer_, 200, 200, 210, 255);
    }
    SDL_RenderClear(renderer_);

    if (blitDemo_ && hasData_) {
        renderBlitDemo(alpha);
    } else if (viewerClip_) {
        RenderContext ctx{renderer_, &symbols_, &textures_};
        ctx.interpolate = settings_.interpolation;
        ctx.alpha = alpha;
        viewerClip_->render(ctx, viewerFit_, ColorTransform{});
    }
    renderOverlay();
    SDL_RenderPresent(renderer_);
}

void App::renderBlitDemo(float alpha)
{
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
        const int frame = 16 + sargeCycle_;
        const float sx = static_cast<float>(frame % perRow) * w;
        const float sy = static_cast<float>(frame / perRow) * h + (sargeDir_ > 0 ? 0.0f : 1280.0f);
        // Original: Math.floor(x - cell / 2). Interpolated: exact sub-pixel position between logic steps.
        const double x = settings_.interpolation ? sargePrevX_ + (sargeX_ - sargePrevX_) * alpha : sargeX_;
        const float dx = settings_.interpolation ? static_cast<float>(x - w / 2) : std::floor(static_cast<float>(x - w / 2));
        const float dy = static_cast<float>(stage_.height) - 96 - 135; // feet on the tiles (spriteTargetY = 135)
        sheets_.blit(*sarge, sx, sy, w, h, dx, dy);
    }
}

void App::renderOverlay()
{
    char cap[16];
    const int originalRate = static_cast<int>(std::lround(stage_.frameRate));
    if (settings_.fpsCap == 0) {
        std::snprintf(cap, sizeof(cap), "max");
    } else if (settings_.fpsCap == originalRate) {
        std::snprintf(cap, sizeof(cap), "original");
    } else {
        std::snprintf(cap, sizeof(cap), "%d", settings_.fpsCap);
    }
    char line[128];
    std::snprintf(line, sizeof(line), "FPS %.0f (%s, display %.0f Hz) | logic %.0f/s | interp %s", static_cast<double>(measuredFps_),
                  cap, static_cast<double>(displayHz_), static_cast<double>(stage_.frameRate),
                  settings_.interpolation ? "ON" : "OFF");
    const SDL_FRect box{2, 2, static_cast<float>(std::strlen(line) * 8 + 4), 12};
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 160);
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(renderer_, &box);
    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
    SDL_RenderDebugText(renderer_, 4, 4, line);
}

} // namespace engine
