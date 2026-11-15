#include "engine/app.h"
#include "engine/paths.h"
#include "game/as3.h"
#include "game/papaGame/managers/CharacterBlitter.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace engine {

namespace {
// Never run more than this many logic frames per display frame (e.g. after a stall or a breakpoint).
constexpr uint64_t kMaxCatchUpFrames = 5;
} // namespace

App::~App()
{
    importScreen_.reset();
    game_.reset();
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

    // Touches are handled as fingers (on-screen buttons, taps); no synthetic mouse events from them.
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMEPAD)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    settings_.load();
#if defined(SDL_PLATFORM_ANDROID)
    input_.touch.visible = true;
#endif

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
    importScreen_ = std::make_unique<ImportScreen>(window_, dataDir_);
    if (hasData_ && fileExists(dataDir_ + "/pl2/symbols.bin")) {
        loadGame("pl2");
    } else {
        mode_ = Mode::Import; // nothing imported yet
    }
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::strcmp(argv[i], "--walk-test") == 0) { // DEV: walk right, jump and attack (--capture checks)
            parseInputScript("R0-199,J31-44,A82-84");
        } else if (std::strcmp(argv[i], "--input") == 0) { // DEV: e.g. "R0-60,U61-90": keys LRUDJA, frames
            parseInputScript(argv[i + 1]);
        } else if (std::strcmp(argv[i], "--trace") == 0) { // DEV: log the player's state every logic frame
            trace_ = true;
            scriptFrame_ = std::max(scriptFrame_, 0);
        } else if (std::strcmp(argv[i], "--frames") == 0) { // DEV: quit after N logic frames
            quitAfterFrames_ = std::atoi(argv[i + 1]);
        } else if (std::strcmp(argv[i], "--touch") == 0) { // DEV: show the on-screen buttons
            input_.touch.visible = true;
        }
        if (std::strcmp(argv[i], "--capture") == 0) {
            capturePath_ = argv[i + 1];
        } else if (std::strcmp(argv[i], "--player-sheet") == 0 && game_) { // DEV: save the player's torso sheet
            papaGame::managers::CharacterBlitter blitter(game_.get());
            blitter.createPlayerSprite();
            for (auto [sheet, suffix] : {std::pair{&blitter.torsoSheet, "torso"}, std::pair{&blitter.frontFootSheet, "front"},
                                         std::pair{&blitter.backFootSheet, "back"}}) {
                if (!sheet->texture) {
                    continue;
                }
                SDL_Texture *previous = SDL_GetRenderTarget(renderer_);
                SDL_SetRenderTarget(renderer_, sheet->texture);
                SDL_Surface *shot = SDL_RenderReadPixels(renderer_, nullptr);
                SDL_SetRenderTarget(renderer_, previous);
                const std::string path = std::string(argv[i + 1]) + "_" + suffix + ".png";
                if (shot) {
                    SDL_SavePNG(shot, path.c_str());
                    SDL_DestroySurface(shot);
                }
                SDL_Log("Player sheet %s: %gx%g cells, %g per row", path.c_str(), blitter.spriteWidth, blitter.spriteHeight,
                        blitter.spritesPerRow);
            }
        } else if (std::strcmp(argv[i], "--ladder") == 0 && game_) {
            // DEV: a ladder "x,y,height" in tiles (bottom tile y), marked like GameObject3 does, until objects exist.
            double lx = 0, ly = 0, lh = 0;
            if (std::sscanf(argv[i + 1], "%lf,%lf,%lf", &lx, &ly, &lh) == 3) {
                for (int k = 0; k < static_cast<int>(lh); ++k) {
                    game_->obf_4i->setProperty(lx, ly - lh + 1 + k, "ladder");
                }
            }
        } else if (std::strcmp(argv[i], "--level") == 0 && game_) { // DEV: start level N (0-based)
            game_->obf_6V(std::atof(argv[i + 1]));
        } else if (std::strcmp(argv[i], "--room") == 0 && game_) { // DEV: start in room N (0-based)
            switchRoom(std::atoi(argv[i + 1]) - static_cast<int>(game_->obf_AQ->currentScreen));
        } else if (std::strcmp(argv[i], "--cam") == 0 && game_) { // DEV: camera "x,y", clamped like a pan
            double x = 0, y = 0;
            if (std::sscanf(argv[i + 1], "%lf,%lf", &x, &y) == 2) {
                auto &cam = *game_->obf_3Y;
                cam.x = std::clamp(x, cam.min_x, std::max(cam.min_x, cam.max_x));
                cam.y = std::clamp(y, cam.min_y, std::max(cam.min_y, cam.max_y));
                cam.savePrevious();
                freeCam_ = true;
            }
        }
    }

    stepNs_ = static_cast<uint64_t>(1e9 / stage_.frameRate);
    lastNs_ = SDL_GetTicksNS();
    fpsWindowStartNs_ = lastNs_;
    startNs_ = lastNs_;
    return SDL_APP_CONTINUE;
}

bool App::captureFrame()
{
    // DEV: save the rendered frame (before present) and quit; used for automated checks.
    SDL_Surface *shot = SDL_RenderReadPixels(renderer_, nullptr);
    const bool ok = shot && SDL_SavePNG(shot, capturePath_.c_str());
    SDL_Log("Capture %s: %s", capturePath_.c_str(), ok ? "saved" : SDL_GetError());
    SDL_DestroySurface(shot);
    return ok;
}

void App::loadGame(const std::string &game)
{
    const std::string dir = dataDir_ + "/" + game;
    game_.reset();
    viewerClip_.reset();
    viewerList_.clear();
    textures_.clear();
    sheets_.clear();
    if (!symbols_.load(dir + "/symbols.bin")) {
        return;
    }
    hasData_ = true;
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

    symbols_.loadFrameScripts(dir + "/framescripts.txt");
    game_ = std::make_unique<obf_9G::obf_4R>(renderer_, &sheets_, &symbols_, &textures_);
    mode_ = Mode::Level;
    if (game_->loadLevelData(dir)) {
        game_->obf_6V(0);
        SDL_Log("Level 0: %s, %zu rooms", game_->obf_AQ->getWorldTitle(0).c_str(),
                game_->obf_AQ->currentScreens.size());
    } else {
        game_.reset();
        mode_ = Mode::BlitDemo;
    }
}

void App::nextMode()
{
    switch (mode_) {
    case Mode::Level: mode_ = Mode::BlitDemo; break;
    case Mode::BlitDemo: mode_ = Mode::Viewer; break;
    case Mode::Viewer: mode_ = Mode::Import; break;
    case Mode::Import: mode_ = game_ ? Mode::Level : (hasData_ ? Mode::BlitDemo : Mode::Import); break;
    }
}

void App::switchRoom(int delta)
{
    if (!game_ || game_->obf_AQ->currentScreens.empty()) {
        return;
    }
    const int n = static_cast<int>(game_->obf_AQ->currentScreens.size());
    const int room = ((static_cast<int>(game_->obf_AQ->currentScreen) + delta) % n + n) % n;
    game_->obf_4X(room, 0, 0, 0, 0, 0); // DEV: enter through the room's first door
}

void App::parseInputScript(const char *script)
{
    // "<keys><from>-<to>,...": keys from LRUDJA (left, right, up, down, jump, attack), logic frames from 1.
    inputScript_.clear();
    for (const std::string &part : as3::split(script, ',')) {
        size_t k = 0;
        std::vector<Action> keys;
        for (; k < part.size() && std::isalpha(static_cast<unsigned char>(part[k])); ++k) {
            switch (part[k]) {
            case 'L': keys.push_back(Action::Left); break;
            case 'R': keys.push_back(Action::Right); break;
            case 'U': keys.push_back(Action::Up); break;
            case 'D': keys.push_back(Action::Down); break;
            case 'J': keys.push_back(Action::Jump); break;
            case 'A': keys.push_back(Action::Attack); break;
            default: break;
            }
        }
        int from = 0, to = 0;
        if (std::sscanf(part.c_str() + k, "%d-%d", &from, &to) == 2) {
            for (Action a : keys) {
                inputScript_.push_back({a, from, to});
            }
        }
    }
    scriptFrame_ = 0;
}

void App::tickLevel()
{
    papaGame::display::GameCam &cam = *game_->obf_3Y;
    cam.savePrevious();
    game_->obf_71->advanceFrame();
    // Actions -> GameControls.keyPressed* (the original sets them from key events).
    auto &player = *game_->playerObj;
    auto &controls = *game_->obf_8o;
    const bool *keys = SDL_GetKeyboardState(nullptr);
    const bool panning = keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT];
    ActionState actions = panning ? ActionState{} : input_.poll();
    if (scriptFrame_ >= 0) {
        ++scriptFrame_;
        for (const ScriptedKey &k : inputScript_) {
            if (scriptFrame_ >= k.from && scriptFrame_ <= k.to) {
                actions.held[static_cast<size_t>(k.action)] = true;
                if (k.action == Action::Jump) {
                    actions.held[static_cast<size_t>(Action::Up)] = true;
                }
            }
        }
    }
    controls.keyPressedLeft = actions[Action::Left];
    controls.keyPressedRight = actions[Action::Right];
    controls.keyPressedUp = actions[Action::Up];
    controls.keyPressedDown = actions[Action::Down];
    controls.keyPressedJump = actions[Action::Jump];
    controls.keyPressedAction = actions[Action::Attack];
    player.savePrevious();
    for (papaGame::models::Enemy *e : game_->obf_E->enemies) {
        if (e) {
            e->savePrevious();
        }
    }
    const double heldX = cam.x, heldY = cam.y;
    controls.detectKeys(); // moves the camera (GameCam.adjustCamera)
    if (trace_ && scriptFrame_ == 1) {
        // DEV: the room's doors (tile x, y, w, h, dirX, dirY) and ladder tiles, to plan --input scripts.
        const auto &dm = *game_->obf_AQ;
        for (const auto &d : dm.currentScreenData->doorArray) {
            std::string row;
            for (double v : d) {
                row += as3::String(v) + " ";
            }
            SDL_Log("DOOR %s", row.c_str());
        }
        const auto &tiles = dm.currentScreenData->tileArray;
        std::string ladders;
        for (size_t ty = 0; ty < tiles.size(); ++ty) {
            for (size_t tx = 0; tx < tiles[ty].size(); ++tx) {
                if (game_->obf_4i->getTileProperty(static_cast<double>(tx), static_cast<double>(ty), "ladder") > 0) {
                    ladders += std::to_string(tx) + "," + std::to_string(ty) + " ";
                }
            }
        }
        SDL_Log("LADDERS %s", ladders.c_str());
    }
    if (trace_) {
        const auto &dm = *game_->obf_AQ;
        SDL_Log("T %d room %.0f x %.1f y %.1f tile %.0f,%.0f %s/%s%s%s%s%s", scriptFrame_, dm.currentScreen, player.x,
                player.y, player.xtile, player.ytile, player.cycleName.c_str(), player.footCycleName.c_str(),
                player.jump ? " jump" : "", player.isClimbingLadder ? " ladder" : "",
                player.waitToEnterDoor ? " door" : "",
                game_->obf_71->isTransitionOut ? " out" : (game_->obf_71->isTransitioningIn ? " in" : ""));
        // DEV: the room's enemies: id:type x,y cycle and flags (d dead, s stunned, h hit).
        std::string enemies;
        for (const papaGame::models::Enemy *e : game_->obf_E->enemies) {
            if (e) {
                char buf[96];
                SDL_snprintf(buf, sizeof(buf), " %.0f:%.0f %.0f,%.0f %s%s%s%s", e->id, e->type, e->x, e->y,
                             e->cycleName.c_str(), e->isDead ? " d" : "", e->isStunned ? " s" : "",
                             e->isHit ? " h" : "");
                enemies += buf;
            }
        }
        if (!enemies.empty()) {
            SDL_Log("E%s", enemies.c_str());
        }
    }
    if (quitAfterFrames_ >= 0 && --quitAfterFrames_ == 0) {
        quitRequested_ = true;
    }
    constexpr double kPan = 12.0;
    double dx = -static_cast<double>(dragX_), dy = -static_cast<double>(dragY_);
    dragX_ = dragY_ = 0.0f;
    if (panning) {
        if (keys[SDL_SCANCODE_LEFT]) dx -= kPan;
        if (keys[SDL_SCANCODE_RIGHT]) dx += kPan;
        if (keys[SDL_SCANCODE_UP]) dy -= kPan;
        if (keys[SDL_SCANCODE_DOWN]) dy += kPan;
    }
    // DEV: Shift + arrows (or a mouse drag) pans freely; on release the game camera chases the player again.
    if (actions[Action::Left] || actions[Action::Right] || actions[Action::Jump] || actions[Action::Down]) {
        freeCam_ = false;
    }
    if (panning || dragging_ || freeCam_) {
        cam.x = std::clamp(heldX + dx, cam.min_x, std::max(cam.min_x, cam.max_x));
        cam.y = std::clamp(heldY + dy, cam.min_y, std::max(cam.min_y, cam.max_y));
    }
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
    input_.event(ev, window_);
    if ((ev.type == SDL_EVENT_FINGER_DOWN || ev.type == SDL_EVENT_FINGER_UP || ev.type == SDL_EVENT_FINGER_MOTION) &&
        mode_ == Mode::Level && input_.touch.event(ev, window_)) {
        return SDL_APP_CONTINUE; // used by an on-screen button
    }
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
            nextMode();
            break;
        case SDLK_LEFTBRACKET:
            switchRoom(-1);
            break;
        case SDLK_RIGHTBRACKET:
            switchRoom(1);
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
            if (mode_ == Mode::Viewer) {
                showSymbol(viewerIndex_ + 1);
            }
            break;
        case SDLK_LEFT:
            if (mode_ == Mode::Viewer) {
                showSymbol(viewerIndex_ - 1);
            }
            break;
        default:
            break;
        }
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        dragging_ = false;
        break;
    case SDL_EVENT_MOUSE_MOTION:
        if (mode_ == Mode::Level && (ev.motion.state & SDL_BUTTON_LMASK)) {
            // Window pixels -> stage pixels.
            int w = 0, h = 0;
            SDL_GetWindowSize(window_, &w, &h);
            const float k = std::max(static_cast<float>(stage_.width) / static_cast<float>(w),
                                     static_cast<float>(stage_.height) / static_cast<float>(h));
            dragX_ += ev.motion.xrel * k;
            dragY_ += ev.motion.yrel * k;
            if (std::fabs(ev.motion.xrel) + std::fabs(ev.motion.yrel) > 0.0f) {
                dragging_ = true;
            }
        }
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (dragging_) {
            dragging_ = false;
            break;
        }
        tap(ev.button.x, ev.button.y);
        break;
    case SDL_EVENT_FINGER_UP: {
        // A touch that did not start on an on-screen button is a tap.
        int w = 0, h = 0;
        SDL_GetWindowSize(window_, &w, &h);
        tap(ev.tfinger.x * static_cast<float>(w), ev.tfinger.y * static_cast<float>(h));
        break;
    }
    default:
        break;
    }
    return SDL_APP_CONTINUE;
}

void App::tap(float winX, float winY)
{
    int w = 0, h = 0;
    SDL_GetWindowSize(window_, &w, &h);
    const float fx = winX / static_cast<float>(w), fy = winY / static_cast<float>(h);
    const int originalRate = static_cast<int>(std::lround(stage_.frameRate));
    if (fy < 0.15f) { // DEV top band: FPS cap | next mode | interpolation
        if (fx < 0.33f) {
            settings_.nextFpsCap(originalRate);
            settings_.save();
        } else if (fx > 0.67f) {
            settings_.interpolation = !settings_.interpolation;
            settings_.save();
        } else {
            nextMode();
        }
    } else if (mode_ == Mode::Import) {
        float sx = 0.0f, sy = 0.0f;
        SDL_RenderCoordinatesFromWindow(renderer_, winX, winY, &sx, &sy);
        importScreen_->tap(sx, sy);
    } else if (mode_ == Mode::Viewer) {
        showSymbol(viewerIndex_ + (fx >= 0.5f ? 1 : -1));
    }
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

    if (importScreen_) {
        const std::string imported = importScreen_->takeFinished();
        if (imported == "pl2") { // SKELETON: only PL2 is playable so far
            loadGame(imported);
        }
    }
    render(static_cast<float>(accumulatorNs_) / static_cast<float>(stepNs_));
    if (quitRequested_) {
        return SDL_APP_SUCCESS;
    }

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
    if (game_ && game_->obf_AQ->currentScreenData) {
        tickLevel();
    }
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

    if (mode_ == Mode::Import) {
        importScreen_->render(renderer_, static_cast<float>(stage_.width), static_cast<float>(stage_.height));
    } else if (mode_ == Mode::Level && game_ && game_->obf_AQ->currentScreenData) {
        game_->obf_71->redrawScreen(alpha, settings_.interpolation);
    } else if (mode_ == Mode::BlitDemo && hasData_) {
        renderBlitDemo(alpha);
    } else if (mode_ == Mode::Viewer && viewerClip_) {
        RenderContext ctx{renderer_, &symbols_, &textures_};
        ctx.interpolate = settings_.interpolation;
        ctx.alpha = alpha;
        viewerClip_->render(ctx, viewerFit_, ColorTransform{});
    }
    if (mode_ == Mode::Level) {
        // The on-screen buttons are drawn in window pixels, anchored to the window corners.
        int lw = 0, lh = 0;
        SDL_RendererLogicalPresentation lmode;
        SDL_GetRenderLogicalPresentation(renderer_, &lw, &lh, &lmode);
        SDL_SetRenderLogicalPresentation(renderer_, 0, 0, SDL_LOGICAL_PRESENTATION_DISABLED);
        int pw = 0, ph = 0;
        SDL_GetRenderOutputSize(renderer_, &pw, &ph);
        input_.touch.render(renderer_, pw, ph);
        SDL_SetRenderLogicalPresentation(renderer_, lw, lh, lmode);
    }
    renderOverlay();
    if (!capturePath_.empty() && SDL_GetTicksNS() - startNs_ > 3000000000ull) {
        captureFrame();
        capturePath_.clear();
        quitRequested_ = true;
    }
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

    if (mode_ == Mode::Level && game_ && game_->obf_AQ->currentScreenData) {
        const auto &dm = *game_->obf_AQ;
        const auto &cam = *game_->obf_3Y;
        std::snprintf(line, sizeof(line), "level %.0f room %.0f/%zu (id %.0f) cam %.0f,%.0f", dm.currentLevel,
                      dm.currentScreen + 1, dm.currentScreens.size(), dm.currentScreenData->roomID, cam.x, cam.y);
        const SDL_FRect box2{2, 16, static_cast<float>(std::strlen(line) * 8 + 4), 12};
        SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 160);
        SDL_RenderFillRect(renderer_, &box2);
        SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
        SDL_RenderDebugText(renderer_, 4, 18, line);
    }
}

} // namespace engine
