#include "game/obf_9G/obf_4R.h"

namespace obf_9G {

obf_4R::obf_4R(SDL_Renderer *renderer_, engine::SheetLibrary *sheets, const engine::SymbolLibrary *symbols_,
               engine::ShapeTextures *textures_, engine::TextLibrary *text_, engine::Audio *audio)
    : renderer(renderer_), symbols(symbols_), textures(textures_), text(text_)
{
    obf_7C = std::make_unique<papaGame::managers::SoundManager>(this, audio);
    obf_51 = std::make_unique<papaGame::data::UserData>(this);
    obf_3A = std::make_unique<papaGame::data::CustomerData>(this);
    // Same order as the original (§_-AP§ creates the DataManager, §_-9k§ the rest).
    obf_AQ = std::make_unique<papaGame::data::DataManager>(this);
    bitmapManager = std::make_unique<papaGame::managers::BitmapManager>(this, sheets);
    obf_4i = std::make_unique<papaGame::managers::MapManager>(this);
    obf_71 = std::make_unique<papaGame::display::GameDisplay>(this);
    obf_3Y = std::make_unique<papaGame::display::GameCam>(this);
    playerObj = std::make_unique<papaGame::models::characters::CustomerChar>(this);
    obf_8o = std::make_unique<papaGame::events::GameControls>(this);
    obf_E = std::make_unique<papaGame::managers::EnemyManager>(this);
    obf_7u = std::make_unique<papaGame::managers::ObjectManager>(this);
    obf_3Z = std::make_unique<papaGame::managers::ItemManager>(this);
    obf_V = std::make_unique<papaGame::managers::EffectManager>(this);
    obf_94 = std::make_unique<papaGame::managers::ChallengeManager>(this);
    obf_7t = std::make_unique<papaGame::events::EventManager>(this);
    obf_5h = engine::MovieClip::createEmpty(*symbols);
}

bool obf_4R::loadLevelData(const std::string &gameDir_)
{
    gameDir = gameDir_;
    obf_AQ->loadTables(gameDir + "/datamanager.json");
    playerObj->loadTables(gameDir + "/customerchar.json");
    obf_E->loadTables(gameDir + "/enemies.json");
    obf_7u->loadTables(gameDir + "/objects.json");
    obf_3Z->loadTables(gameDir + "/items.json");
    obf_V->loadTables(gameDir + "/effects.json");
    obf_94->loadTables(gameDir + "/challenges.json");
    obf_51->loadTables(gameDir + "/userdata.json");
    obf_6Z = std::make_unique<papaGame::models::GameHUD>(this);
    return obf_AQ->prepareLevelData(gameDir + "/levels.xml");
}

// AS: §_-4R§.§_-6V§
void obf_4R::obf_6V(double whichLevel)
{
    obf_AQ->currentLevel = whichLevel;
    obf_8W = whichLevel;
    obf_51->resetLives();
    obf_51->clearTallies();
    // SKELETON: BitmapManager.clearExtraSprites.
    bitmapManager->whichTileset = -1;
    obf_AQ->setupLevelScreens(whichLevel);
    if (obf_AQ->currentScreenData == nullptr) {
        return; // port: no level data
    }
    obf_71->setupPreBlit();
    obf_71->currentXcoord = 0;
    obf_71->currentYcoord = 0;
    obf_3Y->setCameraBounds();
    obf_6Z->updateDisplay();
    obf_6Z->updateTimer();
    // SKELETON: ChallengeManager.resetCurrentChallenges.
    if (!playerObj->blitter) {
        // SKELETON: the original blits the player sprite on the screens before the level starts.
        playerObj->blitter = std::make_unique<papaGame::managers::CharacterBlitter>(this);
        playerObj->blitter->createPlayerSprite();
    }
    const auto &start = obf_AQ->currentScreenData->startPoint;
    if (start.size() >= 3) {
        playerObj->startupPlayer(start[1], start[2], true);
    }
    // SKELETON: challenges and the level intro; the original reaches §_-52§ after the intro transition.
    obf_52();
}

// AS: §_-4R§.§_-52§ (start level)
void obf_4R::obf_52()
{
    obf_3Y->adjustCamera(playerObj->x, playerObj->y, true, true);
    obf_71->currentXcoord = 0;
    obf_71->currentYcoord = 0;
    obf_3Y->savePrevious(); // port: no interpolation across the snap
    obf_AQ->initializeScreenObjects(0);
    obf_7C->startLevelMusic();
    obf_7C->playSound("portalsound_spinfallout.wav");
    // SKELETON: playerObj.startTrainSound (train and river levels).
    obf_6Z->showHUD();
    // SKELETON: setupControls, setupCycleCode, handleCheckpointProgress.
    obf_6Z->updateDisplay();
    obf_6Z->updatePlayerHealth(playerObj->currentHealth);
}

// AS: §_-4R§.§_-4X§ — enter room `roomID` through its door `doorID`.
// The door's offsets are passed along but not read: the player is placed by the door's direction.
void obf_4R::obf_4X(double roomID, double doorID, double /*xOffset*/, double /*yOffset*/, double dirX, double dirY)
{
    obf_E->saveEnemies();
    obf_7u->saveObjects();
    obf_3Z->saveItems();
    // SKELETON: BulletManager.clearBullets.
    obf_V->clearEffects();
    // SKELETON: BitmapManager.clearExtraSprites.
    obf_AQ->setActiveRoom(roomID);
    if (obf_AQ->currentScreenData == nullptr) {
        return;
    }
    obf_71->setupPreBlit();
    obf_71->currentXcoord = 0;
    obf_71->currentYcoord = 0;
    obf_3Y->setCameraBounds();
    obf_6Z->updateDisplay();
    obf_6Z->updateTimer();
    const auto &doors = obf_AQ->currentScreenData->doorArray;
    if (doorID >= 0 && doorID < static_cast<double>(doors.size()) && doors[static_cast<size_t>(doorID)].size() >= 4) {
        const auto &door = doors[static_cast<size_t>(doorID)];
        const double tileX = door[0] + dirX;
        const double tileY = door[1] + (door[3] - 1) + dirY;
        playerObj->startupPlayer(tileX, tileY, false, true);
    }
    obf_3Y->adjustCamera(playerObj->x, playerObj->y, true, true);
    obf_3Y->savePrevious(); // port: no interpolation across the room switch
    obf_71->currentXcoord = 0;
    obf_71->currentYcoord = 0;
    obf_AQ->initializeScreenObjects(0);
    obf_6Z->updateDisplay();
    obf_6Z->showHUD();
    obf_8o->stopCycle = false;
    obf_71->startTransition("in");
}

// AS: §_-4R§.finishLevel
void obf_4R::finishLevel()
{
    SDL_Log("BEAT LEVEL");
    obf_8o->stopCycle = true;
    obf_8o->stopControls = true;
    // SKELETON: playerObj.stopTrainSound (the train and river loops are not played yet).
    obf_AQ->clearCheckpoint();
    // (api.§_-6i§, an analytics call, is left out.)
    if (obf_8o) {
        obf_51->totalTimePlayed += obf_8o->currentLevelTimer;
        SDL_Log("(Updated Time Played)");
    }
    obf_4G();
}

// AS: §_-4R§.§_-4G§
void obf_4R::obf_4G()
{
    obf_6j = std::make_unique<papaGame::screens::StageOutroScreen>(this);
}

// AS: §_-4R§.§_-58§
void obf_4R::obf_58()
{
    if (obf_6j) {
        obf_6j->destroy();
        pendingDestroyOutro_ = std::move(obf_6j); // port: it may be running; freed on the next frame
    }
}

// AS: §_-4R§.§_-4Z§
void obf_4R::obf_4Z(double whichCustomer)
{
    obf_6p = std::make_unique<papaGame::screens::NewCustomerScreen>(this, whichCustomer);
}

// AS: §_-4R§.§_-6s§
void obf_4R::obf_6s()
{
    if (obf_6p) {
        obf_6p->destroy();
        pendingDestroyNewCustomer_ = std::move(obf_6p); // port: it may be running; freed on the next frame
    }
}

// AS: §_-4R§.§_-14§ — SKELETON: ParadeScreen is not ported; the level-complete tally shows instead.
void obf_4R::obf_14(bool /*fromLevel*/)
{
    showApiScreen("EndOfDay");
}

// AS: §_-4R§.§_-Ap§ — the port keeps bitmapManager: it holds the textures of the imported sheets, which the
// next level uses again (the original makes a new one in §_-9k§).
void obf_4R::obf_Ap()
{
    obf_AQ->clearLevelScreens();
    obf_6Z.reset();
    playerObj.reset();
    obf_4i.reset();
    obf_7u.reset();
    obf_E.reset();
    obf_3Z.reset();
    obf_V.reset();
    obf_7t.reset();
    obf_71.reset();
    obf_3Y.reset();
    obf_8o.reset();
}

// AS: §_-4R§.§_-2K§
void obf_4R::obf_2K()
{
    // SKELETON: the game over screen (continue from the checkpoint or quit).
    obf_51->resetLives();
    pendingLevel = obf_AQ->currentLevel;
}

// AS: §_-4R§.§_-9k§ (the managers' tables are read again: in the original they are class constants)
void obf_4R::obf_9k()
{
    obf_V = std::make_unique<papaGame::managers::EffectManager>(this);
    playerObj = std::make_unique<papaGame::models::characters::CustomerChar>(this);
    obf_4i = std::make_unique<papaGame::managers::MapManager>(this);
    obf_7u = std::make_unique<papaGame::managers::ObjectManager>(this);
    obf_E = std::make_unique<papaGame::managers::EnemyManager>(this);
    obf_3Z = std::make_unique<papaGame::managers::ItemManager>(this);
    obf_7t = std::make_unique<papaGame::events::EventManager>(this);
    obf_71 = std::make_unique<papaGame::display::GameDisplay>(this);
    obf_3Y = std::make_unique<papaGame::display::GameCam>(this);
    obf_8o = std::make_unique<papaGame::events::GameControls>(this);
    playerObj->loadTables(gameDir + "/customerchar.json");
    obf_E->loadTables(gameDir + "/enemies.json");
    obf_7u->loadTables(gameDir + "/objects.json");
    obf_3Z->loadTables(gameDir + "/items.json");
    obf_V->loadTables(gameDir + "/effects.json");
    obf_6Z = std::make_unique<papaGame::models::GameHUD>(this);
    obf_8O();
}

// Port: the level again after §_-2K§ (SKELETON: the game over screen), through §_-9k§ and the stage intro.
void obf_4R::runPending()
{
    if (pendingLevel >= 0) {
        obf_8W = pendingLevel;
        pendingLevel = -1;
        obf_9k();
    }
}

// AS: §_-4R§.§_-8O§ — SKELETON: GameDisplay.setupBitmapDraw (the port's display sets itself up per level).
void obf_4R::obf_8O()
{
    obf_Q();
}

// AS: §_-4R§.§_-Q§
void obf_4R::obf_Q()
{
    obf_7L = std::make_unique<papaGame::screens::StageIntroScreen>(this);
}

// AS: §_-4R§.§_-q§ — SKELETON: GameControls.resetKeyFocus (keyboard focus is a Flash matter).
void obf_4R::obf_q()
{
    if (obf_7L) {
        obf_7L->destroy();
        pendingDestroyIntro_ = std::move(obf_7L); // port: it may be running; freed on the next frame
    }
}

// AS: §_-4R§.§_-19§
void obf_4R::obf_19()
{
    obf_3U = std::make_unique<papaGame::screens::GameIntroScreen>(this);
}

// AS: §_-4R§.§_-5a§ — SKELETON: GameControls.resetKeyFocus.
void obf_4R::obf_5a()
{
    if (obf_3U) {
        obf_3U->destroy();
        pendingDestroyStory_ = std::move(obf_3U);
    }
}

void obf_4R::addContainerListener(const std::string &event, std::function<void()> fn)
{
    containerListeners[event] = std::move(fn);
}

void obf_4R::removeContainerListener(const std::string &event)
{
    containerListeners.erase(event);
}

void obf_4R::dispatchContainerEvent(const std::string &event)
{
    auto it = containerListeners.find(event);
    if (it != containerListeners.end()) {
        std::function<void()> fn = it->second; // the handler may remove itself
        fn();
    }
}

void obf_4R::showApiScreen(const std::string &name, const papaGame::screens::MainMenuParams &params)
{
    using Kind = engine::MenuBar::Kind;
    if (splashScreen) {
        splashScreen->destroy();
        pendingDestroySplash_ = std::move(splashScreen);
    }
    if (slotSelectScreen) {
        slotSelectScreen->destroy();
        pendingDestroySlots_ = std::move(slotSelectScreen);
    }
    if (mainMenuScreen) {
        mainMenuScreen->destroy();
        pendingDestroyMenu_ = std::move(mainMenuScreen);
    }
    if (scoreTallyScreen) {
        scoreTallyScreen->destroy();
        pendingDestroyTally_ = std::move(scoreTallyScreen);
    }
    menuBar.clear();
    SDL_Log("API screen: %s", name.c_str());
    apiScreen = name;
    if (name == "SplashScreen") {
        splashScreen = std::make_unique<papaGame::screens::SplashScreen>(this, obf_5h.get());
        engine::MenuBar::Layout layout;
        layout.topVisible = 73;
        layout.bottomVisible = 120;
        menuBar.setup(*symbols, text, lang, "",
                      {{Kind::Action, "menu.play", "clickStart"}, {Kind::Tab, "menu.credits", "clickCredits"}},
                      layout);
    } else if (name == "SlotSelect") {
        slotSelectScreen = std::make_unique<papaGame::screens::SlotSelectScreen>(this, obf_5h.get());
        engine::MenuBar::Layout layout;
        menuBar.setup(*symbols, text, lang, "title.choose_slot", {{Kind::Back, "menu.back", "clickBack"}}, layout);
    } else if (name == "MainMenu") {
        // SKELETON: the BADDIES, BADGES, CONTROLS and HELP tabs (their sections are not ported yet).
        mainMenuScreen = std::make_unique<papaGame::screens::MainMenuScreen>(this, obf_5h.get(), params);
        engine::MenuBar::Layout layout;
        menuBar.setup(*symbols, text, lang, mainMenuScreen->getSectionTitle(),
                      {{Kind::Tab, "menu.credits", "clickCredits"}, {Kind::Back, "menu.quit", "clickExit"}}, layout);
    } else if (name == "MapSelectMenu") {
        mainMenuScreen = std::make_unique<papaGame::screens::MainMenuScreen>(this, obf_5h.get(), params);
        engine::MenuBar::Layout layout;
        menuBar.setup(*symbols, text, lang, mainMenuScreen->getSectionTitle(),
                      {{Kind::Action, "menu.start_level", "clickStartLevel"},
                       {Kind::Tab, "menu.back_to_map", "clickBackToMap"}},
                      layout);
    } else if (name == "EndOfDay") {
        scoreTallyScreen = std::make_unique<papaGame::screens::ScoreTallyScreen>(this, obf_5h.get());
        engine::MenuBar::Layout layout;
        menuBar.setup(*symbols, text, lang, "title.level_complete",
                      {{Kind::Back, "menu.quit", "clickQuit"}, {Kind::Action, "menu.continue", "clickContinue"}},
                      layout);
    } else {
        apiScreen.clear();
    }
}

bool obf_4R::screensActive() const
{
    return !apiScreen.empty() || obf_7L != nullptr || obf_3U != nullptr || obf_6j != nullptr || obf_6p != nullptr;
}

bool obf_4R::updateScreens(const engine::PointerState &pointer)
{
    pendingDestroySplash_.reset();
    pendingDestroyIntro_.reset();
    pendingDestroySlots_.reset();
    pendingDestroyStory_.reset();
    pendingDestroyMenu_.reset();
    pendingDestroyOutro_.reset();
    pendingDestroyTally_.reset();
    pendingDestroyNewCustomer_.reset();
    engine::MovieClip::collectGarbage();
    const bool onBar = menuBar.update(pointer, [this](const std::string &event) { dispatchContainerEvent(event); });
    if (!onBar) {
        engine::MovieClip::dispatchPointer(*obf_5h, engine::Affine{}, pointer);
    }
    obf_5h->advance();
    if (splashScreen) {
        splashScreen->animateScreen();
    }
    if (slotSelectScreen) {
        slotSelectScreen->animateScreen();
    }
    if (mainMenuScreen) {
        mainMenuScreen->enterFrame();
    }
    if (obf_3U) {
        obf_3U->updateScreen();
    }
    if (obf_7L) {
        obf_7L->updateScreen();
    }
    if (obf_6j) {
        obf_6j->updateScreen();
    }
    if (scoreTallyScreen) {
        scoreTallyScreen->updateScreen();
    }
    if (obf_6p) {
        obf_6p->updateScreen();
    }
    return screensActive();
}

void obf_4R::renderScreens(engine::RenderContext &ctx)
{
    ctx.masks = masks;
    obf_5h->render(ctx, engine::Affine{}, engine::ColorTransform{});
    menuBar.render(ctx);
}

} // namespace obf_9G
