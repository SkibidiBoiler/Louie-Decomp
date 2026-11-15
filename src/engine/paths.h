#pragma once
#include <string>

namespace engine {

// Folder holding the converted game assets. They are never bundled with the build:
//   Windows: --data <dir>, else $PAPALOUIE_DATA, else <exe dir>/data
//   Android: <app external files dir>/data (filled by the in-app importer, or adb push)
std::string resolveDataDir(int argc, char *argv[]);

bool directoryExists(const std::string &path);
bool fileExists(const std::string &path);

} // namespace engine
