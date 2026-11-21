#pragma once
// AS: §_-9G§.§_-4R§ — the game object (obfuscated name; it holds every manager and runs the game).
// Ported so far: the managers needed to show a room, the level start (§_-6V§) and the room switch (§_-4X§).
#include <SDL3/SDL.h>
#include <functional>
#include <map>
#include <memory>
#include <string>

#include "engine/audio.h"
#include "engine/lang.h"
#include "engine/menu_bar.h"
#include "engine/movieclip.h"
#include "engine/sheets.h"
#include "engine/symbols.h"
#include "engine/text.h"
#include "engine/textures.h"
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/UserData.h"
#include "game/papaGame/data/DataManager.h"
#include "game/papaGame/display/GameCam.h"
#include "game/papaGame/events/EventManager.h"
#include "game/papaGame/events/GameControls.h"
#include "game/papaGame/display/GameDisplay.h"
#include "game/papaGame/managers/BitmapManager.h"
#include "game/papaGame/managers/BulletManager.h"
#include "game/papaGame/managers/ChallengeManager.h"
#include "game/papaGame/managers/EffectManager.h"
#include "game/papaGame/managers/EnemyManager.h"
#include "game/papaGame/managers/ItemManager.h"
#include "game/papaGame/managers/MapManager.h"
#include "game/papaGame/managers/ObjectManager.h"
#include "game/papaGame/managers/SoundManager.h"
#include "game/papaGame/models/GameHUD.h"
#include "game/papaGame/models/characters/CustomerChar.h"
#include "game/papaGame/screens/GameIntroScreen.h"
#include "game/papaGame/screens/MainMenuScreen.h"
#include "game/papaGame/screens/BossIntroScreen.h"
#include "game/papaGame/screens/FliplineLogoScreen.h"
#include "game/papaGame/screens/GameOutroScreen.h"
#include "game/papaGame/screens/ParadeScreen.h"
#include "game/papaGame/screens/NewCustomerScreen.h"
#include "game/papaGame/screens/ScoreTallyScreen.h"
#include "game/papaGame/screens/StageOutroScreen.h"
#include "game/papaGame/screens/SlotSelectScreen.h"
#include "game/papaGame/screens/SplashScreen.h"
#include "game/papaGame/screens/StageIntroScreen.h"
#include "game/papaGame/screens/TrainingPopup.h"

namespace obf_9G {

class obf_4R {
public:
    // Port: the engine services the translated code draws with.
    obf_4R(SDL_Renderer *renderer, engine::SheetLibrary *sheets, const engine::SymbolLibrary *symbols,
           engine::ShapeTextures *textures, engine::TextLibrary *text, engine::Audio *audio);

    // Port: replaces the level-data loading screen; reads <gameDir>/levels.xml.
    bool loadLevelData(const std::string &gameDir);

    // AS: §_-4R§.§_-6V§ — start level `whichLevel` (so far: rooms, pre-blit and camera bounds).
    void obf_6V(double whichLevel);
    void obf_52();
    // AS: §_-4R§.§_-4X§ — the room-switch part of entering a door (no player or objects yet).
    void obf_4X(double roomID, double doorID, double xOffset, double yOffset, double dirX, double dirY);
    // AS: §_-4R§.finishLevel — "BEAT LEVEL". SKELETON: the level-complete screen; the next level starts instead.
    void finishLevel();
    // AS: §_-4R§.§_-2K§ — out of lives. SKELETON: the game over screen; the level starts again instead.
    void obf_2K();
    void obf_99(); // AS: §_-99§, the level again (died in an elevator)
    // AS: §_-4R§.§_-9k§ — a fresh player, HUD and managers for a level (the original builds them per level).
    void obf_9k();
    // Port: a level change asked for from inside the game loop (finishLevel / §_-2K§ run from GameDisplay) happens
    // at the start of the next logic frame, when no translated object is running.
    void runPending();

    // AS: §_-4R§.§_-8O§ — get ready to play: the stage intro, which starts the level §_-8W§.
    void obf_8O();
    // AS: §_-4R§.§_-Q§ / §_-q§ — create / destroy the StageIntroScreen.
    void obf_Q();
    void obf_q();
    // AS: §_-4R§.§_-19§ / §_-5a§ — create / destroy the GameIntroScreen (the story).
    void obf_4G(); // AS: §_-4G§, the stage outro
    void obf_58(); // AS: §_-58§
    void obf_4Z(double whichCustomer); // AS: §_-4Z§, the new customer screen
    void obf_6s(); // AS: §_-6s§
    void obf_6I(); // AS: §_-6I§ (show the boss intro)
    void obf_7X(); // AS: §_-7X§ (show the Flipline logo)
    void obf_4E(); // AS: §_-4E§ (remove the Flipline logo)
    void obf_3q(); // AS: §_-3q§ (remove the parade)
    void obf_49(); // AS: §_-49§ (show the game ending)
    void obf_2r(); // AS: §_-2r§ (remove the game ending)
    void obf_28(); // AS: §_-28§ (remove the boss intro)
    void obf_14(bool showWeapons = true); // AS: §_-14§, the parade
    void obf_Ap(); // AS: §_-Ap§, the level is torn down
    void obf_19();
    void obf_5a();
    // AS: §_-4R§.§_-6n§ — open the training bubble for `flag` unless it is unknown or already trained.
    bool obf_6n(const std::string &flag, bool isInGame, const std::string &key1 = "", const std::string &key2 = "",
                const std::string &key3 = "", double timeout = -1);
    // AS: §_-4R§.§_-c§ — the training of `flag` is done: remember it and close its bubble.
    bool obf_c(const std::string &flag, bool tween = true);
    // AS: §_-4R§.§_-64§ — close the training bubble (sliding out), or remove it at once.
    void obf_64(bool tween = true);

    // Port: the Flipline API's screen manager is not ported (it was part of the online API). These stand in for
    // the parts of it the game uses: api.§_-1G§(name) switches between its screens (SplashScreen, SlotSelect,
    // MainMenu...), each with its menu bar, and the bar's buttons send events to the screens' container
    // (`container.addEventListener("clickStart", ...)`).
    void showApiScreen(const std::string &name, const papaGame::screens::MainMenuParams &params = {});
    void addContainerListener(const std::string &event, std::function<void()> fn);
    void removeContainerListener(const std::string &event);
    void dispatchContainerEvent(const std::string &event);
    // Port: one logic frame of the screens: the pointer (menu bar, then the screens' buttons), then their
    // ENTER_FRAME handlers. True while a menu screen is up.
    bool updateScreens(const engine::PointerState &pointer);
    engine::PointerState pointer; // port: this frame's pointer (stage mouse position and button, for drags)
    bool waitIntroMusic = true;   // port option: the level intro waits for its music (StageIntroScreen)
    double stageFrameRate = 0;    // port: stage.frameRate as the game sets it (0 = the SWF's); the app follows it
    bool screensActive() const;
    // Port: keyboard text for a screen's text input (the name entry): set while one has focus.
    std::function<void(const std::string &)> onTextInput;
    std::function<void(int key)> onKeyDown; // SDL keycode
    void renderScreens(engine::RenderContext &ctx);
    // Port: the stage's KEY_DOWN listener the CONTROLS tab adds while it waits for a key (Flash key code).
    bool wantsKey() const;
    void keyDown(int keyCode);

    SDL_Renderer *renderer = nullptr; // port
    const engine::SymbolLibrary *symbols = nullptr; // port
    engine::ShapeTextures *textures = nullptr; // port
    engine::TextLibrary *text = nullptr; // port
    std::unique_ptr<papaGame::data::UserData> obf_51;
    std::unique_ptr<papaGame::data::CustomerData> obf_3A;
    std::unique_ptr<papaGame::data::DataManager> obf_AQ;
    std::unique_ptr<papaGame::managers::BitmapManager> bitmapManager;
    std::unique_ptr<papaGame::managers::MapManager> obf_4i;
    std::unique_ptr<papaGame::display::GameDisplay> obf_71;
    std::unique_ptr<papaGame::display::GameCam> obf_3Y;
    std::unique_ptr<papaGame::models::characters::CustomerChar> playerObj;
    std::unique_ptr<papaGame::events::GameControls> obf_8o;
    std::unique_ptr<papaGame::managers::EnemyManager> obf_E;
    std::unique_ptr<papaGame::managers::ObjectManager> obf_7u;
    std::unique_ptr<papaGame::managers::ItemManager> obf_3Z;
    std::unique_ptr<papaGame::managers::EffectManager> obf_V;
    std::unique_ptr<papaGame::managers::BulletManager> obf_4l;
    std::unique_ptr<papaGame::managers::ChallengeManager> obf_94;
    std::unique_ptr<papaGame::events::EventManager> obf_7t;
    std::unique_ptr<papaGame::managers::SoundManager> obf_7C;
    double obf_8W = 0; // AS: §_-8W§, the world (level) being played
    std::string gameDir;      // port: where loadLevelData read the tables
    // Port: §_-2K§ / §_-99§ tear the level down; the display calls them from its own update, so they run at the
    // start of the next frame (runPending).
    enum class Pending { None, GameOver, ResetLevel };
    Pending pending = Pending::None;
    std::unique_ptr<papaGame::models::GameHUD> obf_6Z; // created by loadLevelData, once the tables exist
    std::unique_ptr<engine::MovieClip> obf_5h; // AS: §_-5h§, the container the screens add their clips to
    std::unique_ptr<engine::MovieClip> obf_9X; // AS: §_-9X§, a container above §_-5h§ (no mouse)
    // Port: the clips the original adds to the game object itself later (the badge drop-down), above the others.
    std::unique_ptr<engine::MovieClip> rootClips;
    std::unique_ptr<papaGame::screens::TrainingPopup> obf_a; // AS: §_-a§, the training bubble
    std::unique_ptr<papaGame::screens::StageIntroScreen> obf_7L;
    std::unique_ptr<papaGame::screens::SplashScreen> splashScreen; // port: the API's SplashScreen instance
    std::unique_ptr<papaGame::screens::SlotSelectScreen> slotSelectScreen; // port: the API's SlotSelect
    std::unique_ptr<papaGame::screens::GameIntroScreen> obf_3U;
    std::unique_ptr<papaGame::screens::MainMenuScreen> mainMenuScreen; // port: the API's MainMenu / MapSelectMenu
    std::unique_ptr<papaGame::screens::StageOutroScreen> obf_6j;
    std::unique_ptr<papaGame::screens::NewCustomerScreen> obf_6p;
    std::unique_ptr<papaGame::screens::BossIntroScreen> obf_59;
    std::unique_ptr<papaGame::screens::FliplineLogoScreen> obf_t;
    std::unique_ptr<papaGame::screens::ParadeScreen> obf_5s;
    std::unique_ptr<papaGame::screens::GameOutroScreen> obf_7K;
    std::unique_ptr<papaGame::screens::ScoreTallyScreen> scoreTallyScreen; // port: the API's EndOfDay
    const engine::Lang *lang = nullptr;      // port: the menu bars' texts
    engine::MaskLayers *masks = nullptr;     // port: offscreen layers for masks
    engine::MenuBar menuBar;                 // port: see showApiScreen
    std::string apiScreen;                   // port: the API screen being shown, "" when none
    std::map<std::string, std::function<void()>> containerListeners; // port

private:
    // Port: screens destroyed from their own update are freed at the start of the next frame.
    std::unique_ptr<papaGame::screens::SplashScreen> pendingDestroySplash_;
    std::unique_ptr<papaGame::screens::StageIntroScreen> pendingDestroyIntro_;
    std::unique_ptr<papaGame::screens::SlotSelectScreen> pendingDestroySlots_;
    std::unique_ptr<papaGame::screens::GameIntroScreen> pendingDestroyStory_;
    std::unique_ptr<papaGame::screens::MainMenuScreen> pendingDestroyMenu_;
    std::unique_ptr<papaGame::screens::StageOutroScreen> pendingDestroyOutro_;
    std::unique_ptr<papaGame::screens::ScoreTallyScreen> pendingDestroyTally_;
    std::unique_ptr<papaGame::screens::NewCustomerScreen> pendingDestroyNewCustomer_;
    std::unique_ptr<papaGame::screens::BossIntroScreen> pendingDestroyBossIntro_;
    std::unique_ptr<papaGame::screens::FliplineLogoScreen> pendingDestroyLogo_;
    std::unique_ptr<papaGame::screens::ParadeScreen> pendingDestroyParade_;
    std::unique_ptr<papaGame::screens::GameOutroScreen> pendingDestroyGameOutro_;
};

} // namespace obf_9G
