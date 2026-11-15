cmake_minimum_required(VERSION 3.24)
project(PapaLouieRecomp VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL/CMakeLists.txt" OR
   NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/stb/stb_image.h")
    message(FATAL_ERROR "SDL3 or stb is missing. Run: git submodule update --init --recursive")
endif()

# SDL3 as a shared library: SDLActivity loads libSDL3.so on Android; on Windows SDL3.dll sits next to the exe.
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/SDL EXCLUDE_FROM_ALL)

set(PL_SOURCES
    src/main.cpp
    src/engine/app.cpp
    src/engine/paths.cpp
    src/engine/symbols.cpp
    src/engine/textures.cpp
    src/engine/movieclip.cpp
    src/engine/sheets.cpp
    src/engine/settings.cpp
    src/engine/xml.cpp
    src/import/abc.cpp
    src/import/importer.cpp
    src/import/sha256.cpp
    src/import/swf.cpp
    src/import/symbols_export.cpp
    src/game/obf_9G/obf_4R.cpp
    src/game/papaGame/data/DataManager.cpp
    src/game/papaGame/data/LevelData.cpp
    src/game/papaGame/data/ScreenData.cpp
    src/game/papaGame/data/WorldData.cpp
    src/game/papaGame/display/GameCam.cpp
    src/game/papaGame/display/GameDisplay.cpp
    src/game/papaGame/managers/BitmapManager.cpp
)

if(ANDROID)
    # SDLActivity loads "libmain.so".
    add_library(papalouie SHARED ${PL_SOURCES})
    set_target_properties(papalouie PROPERTIES OUTPUT_NAME main)
else()
    add_executable(papalouie WIN32 ${PL_SOURCES})
endif()

target_include_directories(papalouie PRIVATE src)
target_include_directories(papalouie SYSTEM PRIVATE external/stb)
target_link_libraries(papalouie PRIVATE SDL3::SDL3)

# Desktop command line for the importer (same conversion the game runs on the device).
if(NOT ANDROID)
    add_executable(pl_import src/import/cli.cpp src/import/abc.cpp src/import/importer.cpp src/import/sha256.cpp
                   src/import/swf.cpp src/import/symbols_export.cpp)
    target_include_directories(pl_import PRIVATE src)
    target_include_directories(pl_import SYSTEM PRIVATE external/stb)
    target_link_libraries(pl_import PRIVATE SDL3::SDL3)
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
