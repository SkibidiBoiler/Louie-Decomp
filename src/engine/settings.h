#pragma once
// Port options (not in the original), saved to <pref path>/settings.ini.

#include <string>

namespace engine {

struct Settings {
    // Draw between the previous and current logic step. Off = original look (positions floored, no tweening).
    bool interpolation = true;
    // Render rate cap in frames per second; 0 = as fast as the display (vsync). The game logic always runs at
    // the SWF frame rate, whatever this is set to.
    int fpsCap = 120;
    // Interface language ("en", "es"); empty = the system's.
    std::string language;

    void load();
    void save() const;

    // Cycles Original (SWF rate) -> 60 -> 120 -> display max.
    void nextFpsCap(int originalRate);
};

} // namespace engine
