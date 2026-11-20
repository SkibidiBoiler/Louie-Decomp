#include "engine/settings.h"

#include <SDL3/SDL.h>
#include <cstdio>
#include <cstring>
#include <string>

namespace engine {

namespace {

std::string settingsPath()
{
    char *pref = SDL_GetPrefPath("SkibidiBoiler", "PapaLouieRecomp");
    if (!pref) {
        return {};
    }
    std::string path = std::string(pref) + "settings.ini";
    SDL_free(pref);
    return path;
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
        } else if (float f = 0; std::sscanf(line.c_str(), "touch_opacity=%f", &f) == 1) {
            touchOpacity = f < 0.1f ? 0.1f : f > 1.0f ? 1.0f : f;
        } else if (std::sscanf(line.c_str(), "touch_size=%f", &f) == 1) {
            touchSize = f < 0.5f ? 0.5f : f > 2.0f ? 2.0f : f;
        } else if (std::sscanf(line.c_str(), "show_overlay=%d", &value) == 1) {
            showOverlay = value != 0;
        }
    }
}

void Settings::save() const
{
    const std::string path = settingsPath();
    if (path.empty()) {
        return;
    }
    char text[256];
    const int n = std::snprintf(text, sizeof(text),
                                "interpolation=%d\nfps_cap=%d\nlanguage=%s\ntouch_opacity=%.2f\ntouch_size=%.2f\n"
                                "show_overlay=%d\n",
                                interpolation ? 1 : 0, fpsCap, language.c_str(), static_cast<double>(touchOpacity),
                                static_cast<double>(touchSize), showOverlay ? 1 : 0);
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
