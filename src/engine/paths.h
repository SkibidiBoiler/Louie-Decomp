#pragma once
#include <string>

namespace engine {

// Folder holding the converted game assets. They are never bundled with the build:
//   Windows: --data <dir>, else $LOUIEATTACK_DATA, else <exe dir>/data
//   Android: <app external files dir>/data (filled by the in-app importer, or adb push)
std::string resolveDataDir(int argc, char *argv[]);

// The app's own folder for settings and saved games, ending in a separator ("" if unavailable).
// The first time, saves and settings left by the project's old names ("PapaLouieRecomp", "LouieAttackRecomp") are moved here.
std::string prefDir();

bool directoryExists(const std::string &path);
bool fileExists(const std::string &path);

} // namespace engine
