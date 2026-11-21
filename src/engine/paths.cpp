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
    if (const char *env = SDL_getenv("LOUIEATTACK_DATA")) {
        return env;
    }
    const char *base = SDL_GetBasePath();
    return base ? std::string(base) + "data" : std::string("data");
#endif
}

namespace {

constexpr const char *kOrg = "SkibidiBoiler";
constexpr const char *kApp = "LouieAttackDecomp";
// Earlier names of the project; their saves and settings are adopted on the first run.
constexpr const char *kOldApps[] = {"PapaLouieRecomp", "LouieAttackRecomp"};

// Moves <old>/<entry> to <dir>/<entry> unless <dir> already has one.
void adoptOldEntry(const std::string &old, const std::string &dir, const char *entry)
{
    SDL_PathInfo info;
    const std::string from = old + entry;
    const std::string to = dir + entry;
    if (SDL_GetPathInfo(from.c_str(), &info) && !SDL_GetPathInfo(to.c_str(), &info)) {
        if (!SDL_RenamePath(from.c_str(), to.c_str())) {
            SDL_Log("prefDir: could not move %s: %s", from.c_str(), SDL_GetError());
        }
    }
}

std::string findPrefDir()
{
    char *pref = SDL_GetPrefPath(kOrg, kApp);
    if (!pref) {
        return {};
    }
    std::string dir = pref;
    SDL_free(pref);
    // <...>/SkibidiBoiler/LouieAttackDecomp/ -> <...>/SkibidiBoiler/PapaLouieRecomp/ (same parent folder).
    const size_t appAt = dir.rfind(kApp);
    if (appAt != std::string::npos) {
        for (const char *oldApp : kOldApps) {
            std::string old = dir;
            old.replace(appAt, std::strlen(kApp), oldApp);
            if (directoryExists(old)) {
                adoptOldEntry(old, dir, "saves");
                adoptOldEntry(old, dir, "settings.ini");
            }
        }
    }
    return dir;
}

} // namespace

std::string prefDir()
{
    static const std::string dir = findPrefDir();
    return dir;
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
