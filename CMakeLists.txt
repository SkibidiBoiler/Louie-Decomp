cmake_minimum_required(VERSION 3.24)
project(PapaLouieRecomp VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL/CMakeLists.txt")
    message(FATAL_ERROR "SDL3 is missing. Run: git submodule update --init --recursive")
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
)

if(ANDROID)
    # SDLActivity loads "libmain.so".
    add_library(papalouie SHARED ${PL_SOURCES})
    set_target_properties(papalouie PROPERTIES OUTPUT_NAME main)
else()
    add_executable(papalouie WIN32 ${PL_SOURCES})
endif()

target_include_directories(papalouie PRIVATE src)
target_link_libraries(papalouie PRIVATE SDL3::SDL3)

if(MSVC)
    target_compile_options(papalouie PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(papalouie PRIVATE -Wall -Wextra -Wconversion)
endif()

if(WIN32)
    add_custom_command(TARGET papalouie POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:SDL3::SDL3-shared> $<TARGET_FILE_DIR:papalouie>)
endif()
