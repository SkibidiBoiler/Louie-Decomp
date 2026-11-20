#include "engine/settings.h"

#include "engine/paths.h"

#include <SDL3/SDL.h>
#include <cstdio>
#include <cstring>
#include <string>

namespace engine {

namespace {

std::string settingsPath()
{
    const std::string pref = prefDir();
    return pref.empty() ? std::string() : pref + "settings.ini";
}

} // namespace

void Settings::load()
{
    const std::string path = settingsPath();
    size_t size = 0;
    char *text = path.empty() ? nullptr : static_cast<char *>(SDL_LoadFile(path.c_str(), &size));
    if (!text) {
        return; // first run: defaults
    }
    std::string all(text, size);
    SDL_free(text);
    size_t pos = 0;
    while (pos < all.size()) {
        size_t end = all.find('\n', pos);
        if (end == std::string::npos) {
            end = all.size();
        }
        const std::string line = all.substr(pos, end - pos);
        pos = end + 1;
        int value = 0;
        if (line.rfind("language=", 0) == 0) {
            language = line.substr(9);
            while (!language.empty() && (language.back() == '\r' || language.back() == ' ')) {
                language.pop_back();
            }
        } else if (std::sscanf(line.c_str(), "interpolation=%d", &value) == 1) {
            interpolation = value != 0;
        } else if (std::sscanf(line.c_str(), "fps_cap=%d", &value) == 1) {
            fpsCap = value < 0 ? 0 : value;
        }
    }
}

void Settings::save() const
{
    const std::string path = settingsPath();
    if (path.empty()) {
        return;
    }
    char text[192];
    const int n = std::snprintf(text, sizeof(text), "interpolation=%d\nfps_cap=%d\nlanguage=%s\n",
                                interpolation ? 1 : 0, fpsCap, language.c_str());
    if (!SDL_SaveFile(path.c_str(), text, static_cast<size_t>(n))) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Cannot save settings: %s", SDL_GetError());
    }
}

void Settings::nextFpsCap(int originalRate)
{
    if (fpsCap == originalRate) {
        fpsCap = 60;
    } else if (fpsCap == 60) {
        fpsCap = 120;
    } else if (fpsCap == 120) {
        fpsCap = 0;
    } else {
        fpsCap = originalRate;
    }
}

} // namespace engine
