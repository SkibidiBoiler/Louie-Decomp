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
    launcher_.reset();
    importScreen_.reset();
    game_.reset();
    viewerClip_.reset();
    textures_.clear(); // release textures before the renderer
    text_.clear();
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
    SDL_SetAppMetadata("Louie Attack Decomp", "0.1.0", "io.github.skibidiboiler.louieattack");
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");

    // Touches are handled as fingers (on-screen buttons, taps); no synthetic mouse events from them.
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMEPAD)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    settings_.load();
    options_.applySettings();
    audio_.open();
    lang_.select(settings_.language);
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--customer") == 0 && i + 1 < argc) {
            customer_ = std::atoi(argv[i + 1]);
        }
        for (const char *dev : {"--level", "--room", "--at", "--input", "--trace", "--walk-test", "--customer"}) {
            devDirect_ = devDirect_ || std::strcmp(argv[i], dev) == 0;
        }
        if (std::strcmp(argv[i], "--game") == 0 && i + 1 < argc) { // DEV: skip the game picker ("pl2")
            devGame_ = argv[i + 1];
        }
        for (const char *dev : {"--view", "--player-sheet", "--cam"}) { // DEV views that need a loaded game
            if (std::strcmp(argv[i], dev) == 0 && devGame_.empty()) {
                devGame_ = "pl2";
            }
        }
        if (std::strcmp(argv[i], "--options") == 0) { // DEV: start with the options menu open
            options_.setOpen(true);
        }
        if (std::strcmp(argv[i], "--lang") == 0 && i + 1 < argc) { // DEV: interface language
            lang_.select(argv[i + 1]);
        }
        if (std::strcmp(argv[i], "--tap") == 0 && i + 1 < argc) { // DEV: "x,y@frame;x,y@frame"
            const char *p = argv[i + 1];
            while (*p) {
                ScriptedTap t;
                unsigned long long f = 0;
                if (std::sscanf(p, "%f,%f@%llu", &t.x, &t.y, &f) == 3) {
                    t.frame = f;
                    taps_.push_back(t);
                }
                const char *next = std::strchr(p, ';');
                p = next ? next + 1 : p + std::strlen(p);
            }
        }
        if (std::strcmp(argv[i], "--mute") == 0) {
            audio_.setMasterVolume(0); // DEV: silent automated runs (sounds still play, and --trace lists them)
        }
    }
#if defined(SDL_PLATFORM_ANDROID)
    input_.touch.visible = true;
#endif

    SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
#if defined(SDL_PLATFORM_ANDROID)
    flags |= SDL_WINDOW_FULLSCREEN;
#endif
    int winW = stage_.width * 2, winH = stage_.height * 2;
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::strcmp(argv[i], "--window") == 0) { // DEV: window size, e.g. 1600x720 to look like a phone
            std::sscanf(argv[i + 1], "%dx%d", &winW, &winH);
        }
    }
    if (!SDL_CreateWindowAndRenderer("Louie Attack Decomp", winW, winH, flags,
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
    launcher_ = std::make_unique<Launcher>(lang_, *importScreen_, dataDir_);
    if (devDirect_ && devGame_.empty()) {
        devGame_ = "pl2"; // SKELETON: only PL2 is playable so far
    }
    if (!devGame_.empty() && hasData_ && fileExists(dataDir_ + "/" + devGame_ + "/symbols.bin")) {
        loadGame(devGame_);
    } else {
        enterLauncher(); // the port's first screen
    }
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::strcmp(argv[i], "--walk-test") == 0) { // DEV: walk right, jump and attack (--capture checks)
            parseInputScript("R0-199,J31-44,A82-84");
        } else if (std::strcmp(argv[i], "--input") == 0) { // DEV: e.g. "R0-60,U61-90": keys LRUDJA, frames
            parseInputScript(argv[i + 1]);
        } else if (std::strcmp(argv[i], "--trace") == 0) { // DEV: log the player's state every logic frame
            trace_ = true;
            scriptFrame_ = std::max(scriptFrame_, 0);
        } else if (std::strcmp(argv[i], "--parade") == 0) { // DEV: show the parade (then the ending) at frame N
            paradeFrame_ = std::strtoull(argv[i + 1], nullptr, 10);
        } else if (std::strcmp(argv[i], "--reenter") == 0) { // DEV: start the current level again at frame N
            reenterFrame_ = std::strtoull(argv[i + 1], nullptr, 10);
        } else if (std::strcmp(argv[i], "--frames") == 0) { // DEV: quit after N logic frames
            quitAfterFrames_ = std::atoi(argv[i + 1]);
        } else if (std::strcmp(argv[i], "--touch") == 0) { // DEV: show the on-screen buttons
            input_.touch.visible = true;
        }
        if (std::strcmp(argv[i], "--capture-frame") == 0) {
            captureFrame_ = std::strtoull(argv[i + 1], nullptr, 10);
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
        } else if (std::strcmp(argv[i], "--view") == 0 && !viewerList_.empty()) { // DEV: show a symbol by class
            for (size_t k = 0; k < viewerList_.size(); ++k) {
                if (viewerList_[k].first == argv[i + 1]) {
                    mode_ = Mode::Viewer;
                    showSymbol(static_cast<int>(k));
                }
            }
        } else if (std::strcmp(argv[i], "--level") == 0 && game_) { // DEV: start level N (0-based)
            game_->obf_6V(std::atof(argv[i + 1]));
        } else if (std::strcmp(argv[i], "--room") == 0 && game_) { // DEV: start in room N (0-based)
            switchRoom(std::atoi(argv[i + 1]) - static_cast<int>(game_->obf_AQ->currentScreen));
        } else if (std::strcmp(argv[i], "--at") == 0 && game_) { // DEV: put the player on tile "x,y"
            double tx = 0, ty = 0;
            if (std::sscanf(argv[i + 1], "%lf,%lf", &tx, &ty) == 2) {
                game_->playerObj->startupPlayer(tx, ty, false, true);
                game_->obf_3Y->adjustCamera(game_->playerObj->x, game_->playerObj->y, true, true);
                game_->obf_3Y->savePrevious();
            }
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

void App::enterLauncher()
{
    mode_ = Mode::Launcher;
    launcher_->refresh();
    if (!game_) {
        // The picker's text uses an embedded font of the first imported game (PL2 and PL3 share it).
        for (const char *id : {"pl2", "pl3", "pl1"}) {
            if (fileExists(dataDir_ + "/" + id + "/text.json") && text_.init(renderer_, dataDir_ + "/" + id)) {
                break;
            }
        }
    }
    buildCovers();
}

// The picker's covers: each imported game's title screen (splashScreenMC, as SplashScreen.setupScreen shows it),
// drawn once on the back buffer (so its masks work), read back and kept as a texture. Nothing is saved.
void App::buildCovers()
{
    for (const char *id : {"pl2", "pl3", "pl1"}) {
        if (!launcher_->imported(id) || launcher_->hasCover(id)) {
            continue;
        }
        const std::string dir = dataDir_ + "/" + id;
        SymbolLibrary symbols;
        if (!symbols.load(dir + "/symbols.bin")) {
            continue;
        }
        symbols.loadFrameScripts(dir + "/framescripts.txt");
        std::unique_ptr<MovieClip> clip = MovieClip::create(symbols, "splashScreenMC");
        if (!clip) {
            continue; // not a papaGame title screen (PL1): its box keeps the game's colours
        }
        ShapeTextures textures;
        TextLibrary text;
        textures.init(renderer_, dir, 2);
        text.init(renderer_, dir);
        if (MovieClip *iris = clip->findChild("iris")) {
            iris->gotoAndStop(1);
        }
        if (MovieClip *credits = clip->findChild("credits")) {
            credits->setY(416);
        }
        for (const char *promo : {"promo_holder", "promo2_holder", "promo3_holder"}) {
            if (MovieClip *holder = clip->findChild(promo)) {
                holder->setVisible(false);
            }
        }
        for (int i = 0; i < 60; ++i) {
            clip->advance(); // let the title's intro settle
        }
        SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
        SDL_RenderClear(renderer_);
        RenderContext ctx{renderer_, &symbols, &textures, &text};
        ctx.masks = &masks_;
        clip->render(ctx, Affine{}, ColorTransform{});
        SDL_FRect area{};
        SDL_GetRenderLogicalPresentationRect(renderer_, &area);
        int lw = 0, lh = 0;
        SDL_RendererLogicalPresentation lmode;
        SDL_GetRenderLogicalPresentation(renderer_, &lw, &lh, &lmode);
        SDL_SetRenderLogicalPresentation(renderer_, 0, 0, SDL_LOGICAL_PRESENTATION_DISABLED);
        SDL_Surface *shot = SDL_RenderReadPixels(renderer_, nullptr);
        SDL_SetRenderLogicalPresentation(renderer_, lw, lh, lmode);
        SDL_RenderClear(renderer_);
        clip.reset();
        MovieClip::collectGarbage();
        if (!shot) {
            continue;
        }
        // The art strip between the title screen's black bars: stage y 72..296.
        const float sy = area.h / static_cast<float>(stage_.height);
        SDL_Rect strip{static_cast<int>(area.x), static_cast<int>(area.y + 72 * sy), static_cast<int>(area.w),
                       static_cast<int>(224 * sy)};
        SDL_Surface *banner = SDL_CreateSurface(strip.w, strip.h, shot->format);
        if (banner && SDL_BlitSurface(shot, &strip, banner, nullptr)) {
            SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer_, banner);
            SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_LINEAR);
            launcher_->setCover(id, tex);
            SDL_Log("Cover %s: %dx%d", id, strip.w, strip.h);
        }
        SDL_DestroySurface(banner);
        SDL_DestroySurface(shot);
    }
}

// The selected game's title track while the picker is shown (it needs the game's data).
void App::updateLauncherMusic(float dt)
{
    const std::string game = mode_ == Mode::Launcher ? launcher_->selectedGame() : "";
    if (game != menuMusicGame_) {
        menuMusicGame_ = game;
        if (menuMusicOld_) {
            menuMusicOld_->stop();
        }
        menuMusicOld_ = std::move(menuMusic_);
        menuMusic_.reset();
        if (!game.empty() && launcher_->imported(game)) {
            audio_.setSoundDir(dataDir_ + "/" + game + "/sounds");
            menuMusic_ = audio_.play("TitleTrack", 1 << 30, 0, 0);
        }
    }
    if (menuMusic_) {
        menuMusic_->setTransform(std::min(1.0, menuMusic_->volume() + dt / 0.8), 0);
    }
    if (menuMusicOld_) {
        const double v = menuMusicOld_->volume() - dt / 0.4;
        if (v <= 0) {
            menuMusicOld_->stop();
            menuMusicOld_.reset();
        } else {
            menuMusicOld_->setTransform(v, 0);
        }
    }
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
    text_.init(renderer_, dir);
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
    audio_.setSoundDir(dir + "/sounds");
    game_ = std::make_unique<obf_9G::obf_4R>(renderer_, &sheets_, &symbols_, &textures_, &text_, &audio_);
    mode_ = Mode::Level;
    if (customer_ >= 0) {
        game_->obf_51->selectedCharacter = customer_; // DEV: --customer N (index in CustomerData)
        game_->playerObj->defineVars();
    }
    game_->lang = &lang_;
    game_->masks = &masks_;
    if (game_->loadLevelData(dir)) {
        if (devDirect_) {
            game_->obf_6V(0);
            SDL_Log("Level 0: %s, %zu rooms", game_->obf_AQ->getWorldTitle(0).c_str(),
                    game_->obf_AQ->currentScreens.size());
        } else {
            game_->obf_7X(); // the original starts with the Flipline logo, then the title screen
        }
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
    case Mode::Import: enterLauncher(); break;
    case Mode::Launcher: mode_ = game_ ? Mode::Level : Mode::Import; break;
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
    {
        const auto &u = *game_->obf_51; // the player's keys (CONTROLS tab)
        input_.keyBindings = {static_cast<int>(u.keyCodeLeft), static_cast<int>(u.keyCodeRight),
                              static_cast<int>(u.keyCodeUp),   static_cast<int>(u.keyCodeDown),
                              static_cast<int>(u.keyCodeJump), static_cast<int>(u.keyCodeAttack)};
    }
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
    // AS: keyDownListener — pressing left or right on a ladder hops off it. Port: on the press only (Flash also
    // repeats it with the system's key repeat while the key is held).
    if ((actions[Action::Left] && !controls.keyPressedLeft) || (actions[Action::Right] && !controls.keyPressedRight)) {
        controls.jumpPlayerOnLadder();
    }
    controls.keyPressedLeft = actions[Action::Left];
    controls.keyPressedRight = actions[Action::Right];
    controls.keyPressedUp = actions[Action::Up];
    controls.keyPressedDown = actions[Action::Down];
    controls.keyPressedJump = actions[Action::Jump];
    controls.keyPressedAction = actions[Action::Attack];
    player.savePrevious();
    // Port: every moving thing remembers where it was, for the interpolated render.
    auto savePrevious = [](const auto &list) {
        for (auto *c : list) {
            if (c) {
                c->savePrevious();
            }
        }
    };
    savePrevious(game_->obf_E->enemies);
    savePrevious(game_->obf_7u->objects);
    savePrevious(game_->obf_3Z->items);
    savePrevious(game_->obf_V->effects);
    savePrevious(game_->obf_4l->bullets);
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
        SDL_Log("T %d room %.0f x %.1f y %.1f tile %.0f,%.0f hp %.0f $%.0f sp %.0f %s/%s%s%s%s%s%s%s%s%s%s%s", scriptFrame_,
                dm.currentScreen, player.x,
                player.y, player.xtile, player.ytile, player.currentHealth, game_->obf_51->money,
                game_->obf_51->specialitems, player.cycleName.c_str(), player.footCycleName.c_str(),
                player.jump ? " jump" : "", player.isClimbingLadder ? " ladder" : "",
                player.waitToEnterDoor ? " door" : "",
                game_->obf_71->isTransitionOut ? " out" : (game_->obf_71->isTransitioningIn ? " in" : ""),
                player.isDoubleJumping ? " dj" : "", player.isGliding ? " glide" : "",
                player.isGroundPounding ? " pound" : "", player.isSliding ? " slide" : "",
                player.isWallSliding ? " wallslide" : "", player.isWallJumping ? " walljump" : "");
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
        // DEV: the bullets: id:type x,y frame.
        std::string bullets;
        for (const papaGame::models::Bullet *b : game_->obf_4l->bullets) {
            if (b) {
                char buf[96];
                SDL_snprintf(buf, sizeof(buf), " %.0f:%s %.0f,%.0f f%.0f", b->id, b->bulletType.c_str(), b->x, b->y,
                             b->animationFrame);
                bullets += buf;
            }
        }
        if (!bullets.empty()) {
            SDL_Log("B%s", bullets.c_str());
        }
        // DEV: the steppable objects: id:type x,y frame (r: the player stands on it).
        std::string steppable;
        for (const papaGame::models::GameObject *o : game_->obf_7u->objects) {
            if (o && o->isSteppable) {
                char buf[96];
                SDL_snprintf(buf, sizeof(buf), " %.0f:%.0f %.0f,%.0f f%.0f%s", o->id, o->type, o->x, o->y,
                             o->animationFrame, player.whichMovingTile == o ? " r" : "");
                steppable += buf;
            }
        }
        if (!steppable.empty()) {
            SDL_Log("O%s", steppable.c_str());
        }
        // DEV: the effects: id:type x,y.
        std::string effects;
        for (const papaGame::models::Effect *fx : game_->obf_V->effects) {
            if (fx) {
                char buf[96];
                SDL_snprintf(buf, sizeof(buf), " %.0f:%s %.0f,%.0f", fx->id, fx->effectType.c_str(), fx->x, fx->y);
                effects += buf;
            }
        }
        if (!effects.empty()) {
            SDL_Log("FX%s", effects.c_str());
        }
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
    if ((mode_ == Mode::Level || mode_ == Mode::Launcher) && optionsEvent(ev)) {
        return SDL_APP_CONTINUE; // used by the options menu or its button
    }
    input_.event(ev, window_);
    if (mode_ == Mode::Launcher && launcherEvent(ev)) {
        return SDL_APP_CONTINUE;
    }
    // The pointer in stage coordinates, for the menus (mouse, or the first finger).
    if (ev.type == SDL_EVENT_MOUSE_MOTION || ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
        ev.type == SDL_EVENT_MOUSE_BUTTON_UP || ev.type == SDL_EVENT_FINGER_DOWN || ev.type == SDL_EVENT_FINGER_UP ||
        ev.type == SDL_EVENT_FINGER_MOTION) {
        SDL_Event local = ev;
        SDL_ConvertEventToRenderCoordinates(renderer_, &local);
        const bool finger = ev.type == SDL_EVENT_FINGER_DOWN || ev.type == SDL_EVENT_FINGER_UP ||
                            ev.type == SDL_EVENT_FINGER_MOTION;
        if (finger) {
            pointer_.x = local.tfinger.x;
            pointer_.y = local.tfinger.y;
        } else if (ev.type == SDL_EVENT_MOUSE_MOTION) {
            pointer_.x = local.motion.x;
            pointer_.y = local.motion.y;
        } else {
            pointer_.x = local.button.x;
            pointer_.y = local.button.y;
        }
        pointer_.x -= viewExtra_; // widescreen: logical -> stage coordinates
        const bool isDown = ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN || ev.type == SDL_EVENT_FINGER_DOWN;
        const bool isUp = ev.type == SDL_EVENT_MOUSE_BUTTON_UP || ev.type == SDL_EVENT_FINGER_UP;
        if (isDown && (finger || ev.button.button == SDL_BUTTON_LEFT)) {
            pointer_.down = true;
            pointer_.pressed = true;
        } else if (isUp && (finger || ev.button.button == SDL_BUTTON_LEFT)) {
            pointer_.down = false;
            pointer_.released = true;
        }
    }
    if ((ev.type == SDL_EVENT_FINGER_DOWN || ev.type == SDL_EVENT_FINGER_UP || ev.type == SDL_EVENT_FINGER_MOTION) &&
        mode_ == Mode::Level && input_.touch.event(ev, window_)) {
        return SDL_APP_CONTINUE; // used by an on-screen button
    }
    const int originalRate = static_cast<int>(std::lround(stage_.frameRate));
    if (game_ && game_->onTextInput) {
        // A screen's text input has the keyboard.
        if (ev.type == SDL_EVENT_TEXT_INPUT) {
            game_->onTextInput(ev.text.text);
            return SDL_APP_CONTINUE;
        }
        if (ev.type == SDL_EVENT_KEY_DOWN && ev.key.key != SDLK_ESCAPE && ev.key.key != SDLK_AC_BACK) {
            if (game_->onKeyDown) {
                game_->onKeyDown(static_cast<int>(ev.key.key));
            }
            return SDL_APP_CONTINUE;
        }
    }
    switch (ev.type) {
    case SDL_EVENT_DISPLAY_CURRENT_MODE_CHANGED:
    case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
        // Android switches the panel (e.g. to 120 Hz) after the window opens: keep the shown rate current.
        if (const SDL_DisplayMode *mode = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(window_))) {
            displayHz_ = mode->refresh_rate;
        }
        break;
    case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;
    case SDL_EVENT_KEY_DOWN:
        if (mode_ == Mode::Level && game_ && game_->wantsKey()) { // the CONTROLS tab waits for a key
            game_->keyDown(engine::scancodeToFlashKey(ev.key.scancode));
            break;
        }
        switch (ev.key.key) {
        case SDLK_ESCAPE:
        case SDLK_AC_BACK:
            return SDL_APP_SUCCESS;
        case SDLK_TAB:
            nextMode();
            break;
        case SDLK_P: // port: P opens the pause menu, as the HUD's MENU button does
            if (mode_ == Mode::Level && game_ && !game_->screensActive() && game_->obf_6Z &&
                game_->obf_AQ->currentScreenData) {
                game_->obf_6Z->clickMenu();
            }
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
            const float k = std::max((static_cast<float>(stage_.width) + 2 * viewExtra_) / static_cast<float>(w),
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

// The options menu and its gear button get the pointer and the keys first. Window coordinates are turned into
// the renderer's output pixels, where the menu is drawn.
bool App::optionsEvent(const SDL_Event &ev)
{
    int w = 0, h = 0, pw = 0, ph = 0;
    SDL_GetWindowSize(window_, &w, &h);
    SDL_GetRenderOutputSize(renderer_, &pw, &ph);
    if (w <= 0 || h <= 0) {
        return false;
    }
    const float kx = static_cast<float>(pw) / static_cast<float>(w), ky = static_cast<float>(ph) / static_cast<float>(h);
    options_.setOriginalRate(static_cast<int>(std::lround(stage_.frameRate)));
    switch (ev.type) {
    case SDL_EVENT_KEY_DOWN:
        return options_.key(ev.key.key);
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
    case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
        // Start opens and closes the menu; while it is open the gamepad steers it.
        const bool start = ev.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN && ev.gbutton.button == SDL_GAMEPAD_BUTTON_START;
        if (!start && !options_.isOpen()) {
            return false;
        }
        if (const SDL_Keycode k = gamepadKey(ev)) {
            options_.key(k);
        }
        return true;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        return ev.button.button == SDL_BUTTON_LEFT && options_.pointerDown(ev.button.x * kx, ev.button.y * ky, pw, ph);
    case SDL_EVENT_MOUSE_BUTTON_UP:
        return ev.button.button == SDL_BUTTON_LEFT && options_.pointerUp(ev.button.x * kx, ev.button.y * ky, pw, ph);
    case SDL_EVENT_FINGER_DOWN:
        return options_.pointerDown(ev.tfinger.x * static_cast<float>(pw), ev.tfinger.y * static_cast<float>(ph), pw, ph);
    case SDL_EVENT_FINGER_UP:
        return options_.pointerUp(ev.tfinger.x * static_cast<float>(pw), ev.tfinger.y * static_cast<float>(ph), pw, ph);
    case SDL_EVENT_MOUSE_MOTION:
    case SDL_EVENT_FINGER_MOTION:
        return options_.isOpen();
    default:
        return false;
    }
}

// The gamepad as menu keys: the d-pad and the left stick move, A (south) chooses, B (east) goes back, Start is
// the options menu's key (F1). 0 when the event is not one of those.
SDL_Keycode App::gamepadKey(const SDL_Event &ev)
{
    if (ev.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
        switch (ev.gbutton.button) {
        case SDL_GAMEPAD_BUTTON_DPAD_UP: return SDLK_UP;
        case SDL_GAMEPAD_BUTTON_DPAD_DOWN: return SDLK_DOWN;
        case SDL_GAMEPAD_BUTTON_DPAD_LEFT: return SDLK_LEFT;
        case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: return SDLK_RIGHT;
        case SDL_GAMEPAD_BUTTON_SOUTH: return SDLK_RETURN;
        case SDL_GAMEPAD_BUTTON_EAST: return SDLK_ESCAPE;
        case SDL_GAMEPAD_BUTTON_START: return SDLK_F1;
        default: return 0;
        }
    }
    if (ev.type == SDL_EVENT_GAMEPAD_AXIS_MOTION &&
        (ev.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTX || ev.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTY)) {
        // One step per push: the stick must come back near the centre before it steps again.
        const bool vertical = ev.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTY;
        int &last = vertical ? gamepadAxisY_ : gamepadAxisX_;
        const int v = ev.gaxis.value;
        const int dir = v < -20000 ? -1 : (v > 20000 ? 1 : 0);
        if (dir == 0) {
            if (v > -8000 && v < 8000) {
                last = 0;
            }
            return 0;
        }
        if (dir == last) {
            return 0;
        }
        last = dir;
        if (vertical) {
            return dir < 0 ? SDLK_UP : SDLK_DOWN;
        }
        return dir < 0 ? SDLK_LEFT : SDLK_RIGHT;
    }
    return 0;
}

// The game picker owns the pointer, its keys and the gamepad. Window coordinates become output pixels, as for
// the options menu.
bool App::launcherEvent(const SDL_Event &ev)
{
    int w = 0, h = 0, pw = 0, ph = 0;
    SDL_GetWindowSize(window_, &w, &h);
    SDL_GetRenderOutputSize(renderer_, &pw, &ph);
    if (w <= 0 || h <= 0) {
        return false;
    }
    const float kx = static_cast<float>(pw) / static_cast<float>(w), ky = static_cast<float>(ph) / static_cast<float>(h);
    switch (ev.type) {
    case SDL_EVENT_KEY_DOWN:
        return launcher_->key(ev.key.key);
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
    case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
        const SDL_Keycode k = gamepadKey(ev);
        if (k == SDLK_RETURN || k == SDLK_UP || k == SDLK_DOWN || k == SDLK_LEFT || k == SDLK_RIGHT) {
            launcher_->key(k); // B does not quit the app from here
        }
        return true;
    }
    case SDL_EVENT_MOUSE_MOTION:
        launcher_->pointerMove(ev.motion.x * kx, ev.motion.y * ky, pw, ph);
        return true;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (ev.button.button == SDL_BUTTON_LEFT) {
            launcher_->pointerDown(ev.button.x * kx, ev.button.y * ky, pw, ph);
        }
        return true;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (ev.button.button == SDL_BUTTON_LEFT) {
            launcher_->pointerUp(ev.button.x * kx, ev.button.y * ky, pw, ph);
        }
        return true;
    case SDL_EVENT_FINGER_DOWN:
        launcher_->pointerDown(ev.tfinger.x * static_cast<float>(pw), ev.tfinger.y * static_cast<float>(ph), pw, ph);
        return true;
    case SDL_EVENT_FINGER_UP:
        launcher_->pointerUp(ev.tfinger.x * static_cast<float>(pw), ev.tfinger.y * static_cast<float>(ph), pw, ph);
        return true;
    case SDL_EVENT_FINGER_MOTION:
        return true;
    default:
        return false;
    }
}

// Stage coordinates -> the renderer's output pixels (where the port's own screens are drawn).
void App::stageToPixels(float sx, float sy, float &px, float &py) const
{
    int w = 0, h = 0, pw = 0, ph = 0;
    SDL_GetWindowSize(window_, &w, &h);
    SDL_GetRenderOutputSize(renderer_, &pw, &ph);
    float wx = 0, wy = 0;
    SDL_RenderCoordinatesToWindow(renderer_, sx + viewExtra_, sy, &wx, &wy);
    px = w > 0 ? wx * static_cast<float>(pw) / static_cast<float>(w) : wx;
    py = h > 0 ? wy * static_cast<float>(ph) / static_cast<float>(h) : wy;
}

void App::tap(float winX, float winY)
{
    int w = 0, h = 0;
    SDL_GetWindowSize(window_, &w, &h);
    const float fx = winX / static_cast<float>(w), fy = winY / static_cast<float>(h);
    const int originalRate = static_cast<int>(std::lround(stage_.frameRate));
    if (mode_ == Mode::Level && game_ && game_->screensActive()) {
        return; // the menus take the pointer
    }
    // DEV top band: FPS cap | next mode | interpolation. Only in the DEV modes (viewer, demo, import): in the game and
    // the picker these are set from the options menu, and a tap at the top must not change them.
    if (fy < 0.15f && mode_ != Mode::Level && mode_ != Mode::Launcher) {
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
    const float dt = std::min(0.1f, static_cast<float>(now - lastNs_) / 1e9f);
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
        if (!imported.empty()) {
            hasData_ = true;
            if (mode_ == Mode::Level && game_) {
                launcher_->refresh();
            } else {
                enterLauncher(); // back to the picker, on the game just imported
                launcher_->select(imported);
            }
        }
    }
    if (launcher_) {
        updateLauncherMusic(dt);
        if (mode_ == Mode::Launcher) {
            launcher_->update(dt);
        }
    }
    if (mode_ == Mode::Launcher) {
        if (const std::string chosen = launcher_->takeChosen(); !chosen.empty()) {
            for (auto *music : {&menuMusic_, &menuMusicOld_}) {
                if (*music) {
                    (*music)->stop(); // the game's title screen starts the same track itself
                    music->reset();
                }
            }
            loadGame(chosen); // its title screen
        }
        if (launcher_->quitRequested()) {
            return SDL_APP_SUCCESS;
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
    if (options_.isOpen()) {
        return; // the options menu pauses the game
    }
    ++frame_;
    if (game_ && paradeFrame_ != 0 && frame_ == paradeFrame_) { // DEV: --parade
        game_->obf_14(true);
    }
    if (game_ && reenterFrame_ != 0 && frame_ == reenterFrame_) { // DEV: --reenter (checks the checkpoint load)
        SDL_Log("DEV: enter level %s again", std::to_string(static_cast<int>(game_->obf_AQ->currentLevel)).c_str());
        game_->obf_6V(game_->obf_AQ->currentLevel);
    }
    if (game_) {
        game_->runPending(); // may end the level
    }
    if (game_ && game_->obf_AQ->currentScreenData) {
        tickLevel();
    }
    for (const ScriptedTap &t : taps_) {
        if (mode_ == Mode::Launcher && (frame_ == t.frame || frame_ == t.frame + 2)) {
            float px = 0, py = 0;
            int pw = 0, ph = 0;
            stageToPixels(t.x, t.y, px, py);
            SDL_GetRenderOutputSize(renderer_, &pw, &ph);
            if (frame_ == t.frame) {
                launcher_->pointerDown(px, py, pw, ph);
            } else {
                launcher_->pointerUp(px, py, pw, ph);
            }
            SDL_Log("TAP %.0f,%.0f %s (game picker)", static_cast<double>(t.x), static_cast<double>(t.y),
                    frame_ == t.frame ? "down" : "up");
            continue;
        }
        if (frame_ == t.frame || frame_ == t.frame + 2) {
            pointer_.x = t.x;
            pointer_.y = t.y;
            pointer_.down = frame_ == t.frame;
            (frame_ == t.frame ? pointer_.pressed : pointer_.released) = true;
            SDL_Log("TAP %.0f,%.0f %s", static_cast<double>(t.x), static_cast<double>(t.y),
                    frame_ == t.frame ? "down" : "up");
        } else if (frame_ == t.frame + 1) {
            pointer_.x = t.x;
            pointer_.y = t.y;
        }
    }
    if (game_) {
        // The phone's keyboard comes up while a screen wants text.
        const bool wantsText = static_cast<bool>(game_->onTextInput);
        if (wantsText != SDL_TextInputActive(window_)) {
            if (wantsText) {
                SDL_StartTextInput(window_);
            } else {
                SDL_StopTextInput(window_);
            }
        }
        game_->waitIntroMusic = settings_.waitIntroMusic;
        game_->updateScreens(pointer_);
        if (game_->stageFrameRate > 0 && game_->stageFrameRate != stage_.frameRate) { // stage.frameRate = n
            stage_.frameRate = game_->stageFrameRate;
            stepNs_ = static_cast<uint64_t>(1e9 / stage_.frameRate);
        }
        game_->obf_7C->advanceTime(1000.0 / stage_.frameRate);
        if (game_->wantsFrontend) { // port: the main menu's QUIT chose the game picker
            game_.reset();
            stage_.frameRate = 30;
            stepNs_ = static_cast<uint64_t>(1e9 / stage_.frameRate);
            enterLauncher();
        }
    }
    pointer_.pressed = false;
    pointer_.released = false;
    if (quitAfterFrames_ >= 0 && --quitAfterFrames_ == 0) {
        quitRequested_ = true; // DEV: --frames N counts app logic frames (menus too)
    }
    audio_.update();
    if (trace_) {
        std::string sounds;
        for (const std::string &name : audio_.takeStarted()) {
            sounds += " " + name;
        }
        if (!sounds.empty()) {
            SDL_Log("S%s", sounds.c_str()); // DEV: sounds started this frame
        }
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

void App::updatePresentation()
{
    float extra = 0;
    int pw = 0, ph = 0;
    SDL_GetRenderOutputSize(renderer_, &pw, &ph);
    // Menus over the level (pause, results) have no art past the stage: they keep the original framing.
    if (settings_.widescreen && mode_ == Mode::Level && game_ && game_->obf_AQ->currentScreenData &&
        !game_->screensActive() && ph > 0) {
        // Whole logical pixels each side; at most 2.6:1 (wider windows get the rest as bars).
        const float wide = static_cast<float>(stage_.height) * static_cast<float>(pw) / static_cast<float>(ph);
        extra = std::clamp(std::floor((wide - static_cast<float>(stage_.width)) / 2), 0.0f, 190.0f);
    }
    if (game_) {
        game_->obf_71->viewExtraX = extra;
    }
    if (extra == viewExtra_) {
        return;
    }
    viewExtra_ = extra;
    SDL_SetRenderLogicalPresentation(renderer_, stage_.width + 2 * static_cast<int>(extra), stage_.height,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);
}

void App::render(float alpha)
{
    if (!settings_.interpolation) {
        alpha = 1.0f; // original look: always the latest logic state
    }
    updatePresentation();
    if (!hasData_) {
        SDL_SetRenderDrawColor(renderer_, 110, 28, 28, 255); // red: data folder missing
    } else {
        SDL_SetRenderDrawColor(renderer_, 200, 200, 210, 255);
    }
    SDL_RenderClear(renderer_);

    if (mode_ == Mode::Import) {
        importScreen_->render(renderer_, static_cast<float>(stage_.width), static_cast<float>(stage_.height));
    } else if (mode_ == Mode::Level && game_) {
        if (game_->obf_AQ->currentScreenData) {
            game_->obf_71->redrawScreen(alpha, settings_.interpolation);
        }
        RenderContext ctx{renderer_, &symbols_, &textures_, &text_};
        ctx.interpolate = settings_.interpolation;
        ctx.alpha = alpha;
        game_->renderScreens(ctx, viewExtra_);
    } else if (mode_ == Mode::BlitDemo && hasData_) {
        renderBlitDemo(alpha);
    } else if (mode_ == Mode::Viewer && viewerClip_) {
        RenderContext ctx{renderer_, &symbols_, &textures_, &text_};
        ctx.interpolate = settings_.interpolation;
        ctx.alpha = alpha;
        viewerClip_->render(ctx, viewerFit_, ColorTransform{});
    }
    if (mode_ == Mode::Level || mode_ == Mode::Launcher) {
        renderWindowLayer();
    }
    if (settings_.showOverlay) {
        renderOverlay();
    }
    const bool captureNow = captureFrame_ > 0 ? frame_ >= captureFrame_ : SDL_GetTicksNS() - startNs_ > 3000000000ull;
    if (!capturePath_.empty() && captureNow) {
        captureFrame();
        capturePath_.clear();
        quitRequested_ = true;
    }
    SDL_RenderPresent(renderer_);
}

void App::renderWindowLayer()
{
    // Drawn in window pixels: the touch buttons anchored to the window corners, the game picker filling it.
    int lw = 0, lh = 0;
    SDL_RendererLogicalPresentation lmode;
    SDL_GetRenderLogicalPresentation(renderer_, &lw, &lh, &lmode);
    SDL_SetRenderLogicalPresentation(renderer_, 0, 0, SDL_LOGICAL_PRESENTATION_DISABLED);
    int pw = 0, ph = 0;
    SDL_GetRenderOutputSize(renderer_, &pw, &ph);
    if (mode_ == Mode::Launcher) {
        launcher_->render(renderer_, text_, pw, ph);
    } else {
        input_.touch.render(renderer_, pw, ph);
    }
    options_.setOriginalRate(static_cast<int>(std::lround(stage_.frameRate)));
    options_.render(renderer_, text_, pw, ph);
    SDL_SetRenderLogicalPresentation(renderer_, lw, lh, lmode);
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
