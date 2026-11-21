cmake_minimum_required(VERSION 3.24)
project(PapaLouieRecomp VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL/CMakeLists.txt" OR
   NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/stb/stb_image.h" OR
   NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/plutovg/CMakeLists.txt" OR
   NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/minimp3/minimp3.h")
    message(FATAL_ERROR "A submodule (SDL3, stb, PlutoVG, minimp3) is missing. Run: git submodule update --init --recursive")
endif()

# SDL3 as a shared library: SDLActivity loads libSDL3.so on Android; on Windows SDL3.dll sits next to the exe.
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/SDL EXCLUDE_FROM_ALL)

# PlutoVG (MIT; rasteriser parts under the FreeType licence): the importer's vector rasteriser.
set(PLUTOVG_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(external/plutovg EXCLUDE_FROM_ALL)
if(NOT MSVC)
    # The importer runs on the phone: keep it fast even in debug builds.
    target_compile_options(plutovg PRIVATE -O2)
    set_source_files_properties(src/import/raster.cpp src/import/swf.cpp src/import/symbols_export.cpp
                                src/import/sound_export.cpp
                                PROPERTIES COMPILE_OPTIONS "-O2")
endif()

# Sheet lists (class names baked into sheets) compiled into the importer.
set(PL_GENERATED ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/tools/sheets/pl2.txt PL_SHEETS_PL2)
file(WRITE ${PL_GENERATED}/sheet_lists.h.tmp
     "#pragma once
// Generated from tools/sheets/*.txt by CMake.
static const char kSheetsPl2[] = R\"PLSHEETS(${PL_SHEETS_PL2})PLSHEETS\";
")
file(COPY_FILE ${PL_GENERATED}/sheet_lists.h.tmp ${PL_GENERATED}/sheet_lists.h ONLY_IF_DIFFERENT)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tools/sheets/pl2.txt)

# The port's translation tables (lang/*.json), compiled in.
file(GLOB PL_LANG_FILES ${CMAKE_CURRENT_SOURCE_DIR}/lang/*.json)
set(PL_LANG_HEADER "#pragma once
// Generated from lang/*.json by CMake.
struct LangTable { const char *code; const char *json; };
static const LangTable kLangTables[] = {
")
foreach(lang_file ${PL_LANG_FILES})
    get_filename_component(lang_code ${lang_file} NAME_WE)
    file(READ ${lang_file} lang_json)
    string(APPEND PL_LANG_HEADER "    {\"${lang_code}\", R\"PLLANG(${lang_json})PLLANG\"},
")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${lang_file})
endforeach()
string(APPEND PL_LANG_HEADER "};
")
file(WRITE ${PL_GENERATED}/lang_tables.h.tmp "${PL_LANG_HEADER}")
file(COPY_FILE ${PL_GENERATED}/lang_tables.h.tmp ${PL_GENERATED}/lang_tables.h ONLY_IF_DIFFERENT)

set(PL_SOURCES
    src/main.cpp
    src/engine/app.cpp
    src/engine/audio.cpp
    src/engine/import_screen.cpp
    src/engine/input.cpp
    src/engine/paths.cpp
    src/engine/symbols.cpp
    src/engine/text.cpp
    src/engine/textures.cpp
    src/engine/movieclip.cpp
    src/engine/sheets.cpp
    src/engine/settings.cpp
    src/engine/options_menu.cpp
    src/engine/json.cpp
    src/engine/lang.cpp
    src/engine/menu_bar.cpp
    src/engine/shared_object.cpp
    src/engine/xml.cpp
    src/import/abc.cpp
    src/import/importer.cpp
    src/import/raster.cpp
    src/import/sha256.cpp
    src/import/swf.cpp
    src/import/symbols_export.cpp
    src/import/sound_export.cpp
    src/game/obf_9G/obf_4R.cpp
    src/game/papaGame/data/Challenge.cpp
    src/game/papaGame/data/CustomerData.cpp
    src/game/papaGame/data/CheckpointData.cpp
    src/game/papaGame/data/DataManager.cpp
    src/game/papaGame/data/LevelData.cpp
    src/game/papaGame/data/ScreenData.cpp
    src/game/papaGame/data/UserData.cpp
    src/game/papaGame/data/WorldData.cpp
    src/game/papaGame/display/GameCam.cpp
    src/game/papaGame/events/EventManager.cpp
    src/game/papaGame/events/GameControls.cpp
    src/game/papaGame/display/GameDisplay.cpp
    src/game/papaGame/managers/BitmapManager.cpp
    src/game/papaGame/managers/BulletManager.cpp
    src/game/papaGame/managers/CharacterBlitter.cpp
    src/game/papaGame/managers/ChallengeManager.cpp
    src/game/papaGame/managers/EffectManager.cpp
    src/game/papaGame/managers/EnemyManager.cpp
    src/game/papaGame/managers/ItemManager.cpp
    src/game/papaGame/managers/MapManager.cpp
    src/game/papaGame/managers/ObjectManager.cpp
    src/game/papaGame/managers/SoundManager.cpp
    src/game/papaGame/models/Character.cpp
    src/game/papaGame/models/Bullet.cpp
    src/game/papaGame/models/Effect.cpp
    src/game/papaGame/models/bullets/Banana.cpp
    src/game/papaGame/models/bullets/Basketball.cpp
    src/game/papaGame/models/bullets/BlueberryShot.cpp
    src/game/papaGame/models/bullets/ChiliCan.cpp
    src/game/papaGame/models/bullets/DogToy.cpp
    src/game/papaGame/models/bullets/PepperBomb.cpp
    src/game/papaGame/models/bullets/SargePepper.cpp
    src/game/papaGame/models/bullets/ShroomArrow.cpp
    src/game/papaGame/models/bullets/CheeseBullet.cpp
    src/game/papaGame/models/bullets/SauceBullet.cpp
    src/game/papaGame/models/bullets/Beaker.cpp
    src/game/papaGame/models/bullets/BeakerBlob.cpp
    src/game/papaGame/models/bullets/GiantPepper.cpp
    src/game/papaGame/models/bullets/RomanCandle.cpp
    src/game/papaGame/models/bullets/XandraStar.cpp
    src/game/papaGame/models/bullets/GummyOnion.cpp
    src/game/papaGame/models/bullets/Cookie.cpp
    src/game/papaGame/models/effects/CageBackBarsEffect.cpp
    src/game/papaGame/models/effects/CageCustomerEffect.cpp
    src/game/papaGame/models/effects/CageFrontBarsEffect.cpp
    src/game/papaGame/models/effects/CageSpinnerEffect.cpp
    src/game/papaGame/models/effects/BBQBitEffect.cpp
    src/game/papaGame/models/effects/BlueberryBitEffect.cpp
    src/game/papaGame/models/effects/BlueberrySplatEffect.cpp
    src/game/papaGame/models/effects/CheeseBitEffect.cpp
    src/game/papaGame/models/effects/CheeseSplatEffect.cpp
    src/game/papaGame/models/effects/SauceBitEffect.cpp
    src/game/papaGame/models/effects/SauceSplatEffect.cpp
    src/game/papaGame/models/effects/RainbowBitEffect.cpp
    src/game/papaGame/models/effects/BeakerGlassEffect.cpp
    src/game/papaGame/models/effects/BossPropellerEffect.cpp
    src/game/papaGame/models/effects/RadishDirtEffect.cpp
    src/game/papaGame/models/effects/StuckCheeseEffect.cpp
    src/game/papaGame/models/effects/ChiliBitEffect.cpp
    src/game/papaGame/models/effects/ChiliSplatEffect.cpp
    src/game/papaGame/models/effects/CookieBitEffect.cpp
    src/game/papaGame/models/effects/FeatherEffect.cpp
    src/game/papaGame/models/effects/SmokeRingEffect.cpp
    src/game/papaGame/models/effects/CrackerBreakEffect.cpp
    src/game/papaGame/models/effects/DizzyEffect.cpp
    src/game/papaGame/models/effects/DoorFrontEffect.cpp
    src/game/papaGame/models/effects/DustEffect.cpp
    src/game/papaGame/models/effects/BossEscapeEffect.cpp
    src/game/papaGame/models/effects/LaughEffect.cpp
    src/game/papaGame/models/effects/EndPortalEffect.cpp
    src/game/papaGame/models/effects/ExplosionEffect.cpp
    src/game/papaGame/models/effects/ItemBalloonEffect.cpp
    src/game/papaGame/models/effects/ItemCoinEffect.cpp
    src/game/papaGame/models/effects/ItemDaisyEffect.cpp
    src/game/papaGame/models/effects/ItemFizzoCanEffect.cpp
    src/game/papaGame/models/effects/ItemGoldHelmetEffect.cpp
    src/game/papaGame/models/effects/ItemGummyWormEffect.cpp
    src/game/papaGame/models/effects/ItemHeartEffect.cpp
    src/game/papaGame/models/effects/ItemPapaCoinEffect.cpp
    src/game/papaGame/models/effects/ItemRadishCoinEffect.cpp
    src/game/papaGame/models/effects/ItemSargeCoinEffect.cpp
    src/game/papaGame/models/effects/PlayerCheckpointEffect.cpp
    src/game/papaGame/models/effects/RomanSparkEffect.cpp
    src/game/papaGame/models/effects/RomanTrailEffect.cpp
    src/game/papaGame/models/effects/StarEffect.cpp
    src/game/papaGame/models/effects/StarburstEffect.cpp
    src/game/papaGame/models/effects/ZillaPartsEffect.cpp
    src/game/papaGame/models/Enemy.cpp
    src/game/papaGame/models/GameHUD.cpp
    src/game/papaGame/models/GameItem.cpp
    src/game/papaGame/models/GameObject.cpp
    src/game/papaGame/models/items/GameItem_Balloon.cpp
    src/game/papaGame/models/items/GameItem_Coin.cpp
    src/game/papaGame/models/items/GameItem_Daisy.cpp
    src/game/papaGame/models/items/GameItem_FizzoCan.cpp
    src/game/papaGame/models/items/GameItem_GoldHelmet.cpp
    src/game/papaGame/models/items/GameItem_GummyWorm.cpp
    src/game/papaGame/models/items/GameItem_Heart.cpp
    src/game/papaGame/models/items/GameItem_PapaCoin.cpp
    src/game/papaGame/models/items/GameItem_RadishCoin.cpp
    src/game/papaGame/models/items/GameItem_SargeCoin.cpp
    src/game/papaGame/models/objects/GameObject3.cpp
    src/game/papaGame/models/objects/GameObject33.cpp
    src/game/papaGame/models/objects/GameObject29.cpp
    src/game/papaGame/models/objects/GameObject30.cpp
    src/game/papaGame/models/objects/GameObject31.cpp
    src/game/papaGame/models/objects/GameObject32.cpp
    src/game/papaGame/models/objects/GameObject4.cpp
    src/game/papaGame/models/objects/GameObject5.cpp
    src/game/papaGame/models/objects/GameObject6.cpp
    src/game/papaGame/models/objects/GameObject7.cpp
    src/game/papaGame/models/objects/GameObject8.cpp
    src/game/papaGame/models/objects/GameObject9.cpp
    src/game/papaGame/models/objects/GameObject10.cpp
    src/game/papaGame/models/objects/GameObject11.cpp
    src/game/papaGame/models/objects/GameObject12.cpp
    src/game/papaGame/models/objects/GameObject13.cpp
    src/game/papaGame/models/objects/GameObject1.cpp
    src/game/papaGame/models/objects/GameObject14.cpp
    src/game/papaGame/models/objects/GameObject15.cpp
    src/game/papaGame/models/objects/GameObject16.cpp
    src/game/papaGame/models/objects/GameObject17.cpp
    src/game/papaGame/models/objects/GameObject18.cpp
    src/game/papaGame/models/objects/GameObject19.cpp
    src/game/papaGame/models/objects/GameObject20.cpp
    src/game/papaGame/models/objects/GameObject21.cpp
    src/game/papaGame/models/objects/GameObject22.cpp
    src/game/papaGame/models/objects/GameObject23.cpp
    src/game/papaGame/models/objects/GameObject24.cpp
    src/game/papaGame/models/objects/GameObject25.cpp
    src/game/papaGame/models/objects/GameObject26.cpp
    src/game/papaGame/models/objects/GameObject27.cpp
    src/game/papaGame/models/objects/GameObject28.cpp
    src/game/papaGame/models/objects/GameObject2.cpp
    src/game/papaGame/models/enemies/Enemy1.cpp
    src/game/papaGame/models/enemies/Enemy10.cpp
    src/game/papaGame/models/enemies/Enemy11.cpp
    src/game/papaGame/models/enemies/Enemy5.cpp
    src/game/papaGame/models/enemies/Enemy9.cpp
    src/game/papaGame/models/enemies/Enemy2.cpp
    src/game/papaGame/models/enemies/Enemy6.cpp
    src/game/papaGame/models/enemies/Enemy3.cpp
    src/game/papaGame/models/enemies/Enemy4.cpp
    src/game/papaGame/models/enemies/Enemy7.cpp
    src/game/papaGame/models/enemies/Enemy8.cpp
    src/game/papaGame/models/enemies/Enemy12.cpp
    src/game/papaGame/models/enemies/Enemy14.cpp
    src/game/papaGame/models/enemies/Enemy15.cpp
    src/game/papaGame/models/enemies/Enemy16.cpp
    src/game/papaGame/models/enemies/Enemy17.cpp
    src/game/papaGame/models/enemies/Enemy13.cpp
    src/game/papaGame/models/enemies/Enemy18.cpp
    src/game/papaGame/models/enemies/Enemy19.cpp
    src/game/papaGame/models/enemies/Enemy20.cpp
    src/game/papaGame/models/enemies/Enemy21.cpp
    src/game/papaGame/models/enemies/Enemy22.cpp
    src/game/papaGame/models/enemies/Enemy23.cpp
    src/game/papaGame/models/enemies/Enemy24.cpp
    src/game/papaGame/models/enemies/Enemy25.cpp
    src/game/papaGame/models/enemies/Enemy26.cpp
    src/game/papaGame/models/enemies/Enemy27.cpp
    src/game/papaGame/models/enemies/Enemy28.cpp
    src/game/papaGame/models/enemies/Enemy29.cpp
    src/game/papaGame/models/enemies/Enemy30.cpp
    src/game/papaGame/models/enemies/Enemy31.cpp
    src/game/papaGame/models/enemies/Enemy32.cpp
    src/game/papaGame/models/enemies/Enemy33.cpp
    src/game/papaGame/models/enemies/Enemy35.cpp
    src/game/papaGame/models/enemies/Enemy36.cpp
    src/game/papaGame/models/enemies/Enemy37.cpp
    src/game/papaGame/models/enemies/Enemy38.cpp
    src/game/papaGame/models/enemies/Enemy39.cpp
    src/game/papaGame/models/enemies/Enemy34.cpp
    src/game/papaGame/models/characters/CustomerChar.cpp
    src/game/papaGame/screens/GameIntroScreen.cpp
    src/game/papaGame/screens/MainMenuScreen.cpp
    src/game/papaGame/screens/NewCustomerScreen.cpp
    src/game/papaGame/screens/ScoreTallyScreen.cpp
    src/game/papaGame/screens/StageOutroScreen.cpp
    src/game/papaGame/screens/TrainingPopup.cpp
    src/game/papaGame/screens/SlotSelectScreen.cpp
    src/game/papaGame/screens/SplashScreen.cpp
    src/game/papaGame/screens/StageIntroScreen.cpp
)

if(ANDROID)
    # SDLActivity loads "libmain.so".
    add_library(papalouie SHARED ${PL_SOURCES})
    set_target_properties(papalouie PROPERTIES OUTPUT_NAME main)
else()
    add_executable(papalouie WIN32 ${PL_SOURCES})
endif()

target_include_directories(papalouie PRIVATE src)
target_include_directories(papalouie SYSTEM PRIVATE external/stb external/minimp3 ${PL_GENERATED})
target_link_libraries(papalouie PRIVATE SDL3::SDL3 plutovg::plutovg)

# Desktop command line for the importer (same conversion the game runs on the device).
if(NOT ANDROID)
    add_executable(pl_import src/import/cli.cpp src/import/abc.cpp src/import/importer.cpp src/import/raster.cpp
                   src/import/sha256.cpp src/import/swf.cpp src/import/symbols_export.cpp
                   src/import/sound_export.cpp)
    target_include_directories(pl_import PRIVATE src)
    target_include_directories(pl_import SYSTEM PRIVATE external/stb external/minimp3 ${PL_GENERATED})
    target_link_libraries(pl_import PRIVATE SDL3::SDL3 plutovg::plutovg)
    if(MSVC)
        target_compile_options(pl_import PRIVATE /W4 /permissive- /utf-8)
        target_compile_definitions(pl_import PRIVATE _CRT_SECURE_NO_WARNINGS)
    else()
        target_compile_options(pl_import PRIVATE -Wall -Wextra -Wconversion)
    endif()
endif()

if(MSVC)
    target_compile_options(papalouie PRIVATE /W4 /permissive- /utf-8)
    target_compile_definitions(papalouie PRIVATE _CRT_SECURE_NO_WARNINGS) # sscanf with bounded %Ns is fine
else()
    target_compile_options(papalouie PRIVATE -Wall -Wextra -Wconversion)
endif()

if(WIN32)
    add_custom_command(TARGET papalouie POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:SDL3::SDL3-shared> $<TARGET_FILE_DIR:papalouie>)
endif()
