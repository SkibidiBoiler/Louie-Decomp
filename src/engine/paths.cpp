#include "engine/paths.h"

#include <SDL3/SDL.h>
#include <cstring>

namespace engine {

std::string resolveDataDir(int argc, char *argv[])
{
#if defined(SDL_PLATFORM_ANDROID)
    (void)argc;
    (void)argv;
    const char *external = SDL_GetAndroidExternalStoragePath();
    return external ? std::string(external) + "/data" : std::string();
#else
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::strcmp(argv[i], "--data") == 0) {
            return argv[i + 1];
        }
    }
    if (const char *env = SDL_getenv("PAPALOUIE_DATA")) {
        return env;
    }
    const char *base = SDL_GetBasePath();
    return base ? std::string(base) + "data" : std::string("data");
#endif
}

bool directoryExists(const std::string &path)
{
    SDL_PathInfo info;
    return !path.empty() && SDL_GetPathInfo(path.c_str(), &info) && info.type == SDL_PATHTYPE_DIRECTORY;
}

bool fileExists(const std::string &path)
{
    SDL_PathInfo info;
    return !path.empty() && SDL_GetPathInfo(path.c_str(), &info) && info.type == SDL_PATHTYPE_FILE;
}

} // namespace engine
