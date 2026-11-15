#pragma once
// In-app importer: converts the owner's .swf into the port's own formats (docs/asset-format.md) in the data
// folder, on the device itself. Runs on a worker thread; reports progress through a callback.
#include <atomic>
#include <functional>
#include <string>

namespace swfimport {

struct Progress {
    float fraction = 0; // 0..1
    std::string step;
};

struct GameInfo {
    const char *id;    // data folder name: "pl1", "pl2", "pl3"
    const char *title;
    const char *sha256Prefix; // the copy the port was made from (other copies may still work)
};

// The games the port knows, in the order they are offered.
const GameInfo *knownGames(int &count);

class Importer {
public:
    using ProgressFn = std::function<void(const Progress &)>;

    // Converts `swfPath` into `dataDir`/<game id>/. `expectedGame` (optional) is the game the user picked;
    // the import fails if the file is a different game.
    bool run(const std::string &swfPath, const std::string &dataDir, const ProgressFn &progress,
             const std::string &expectedGame = {});

    const std::string &error() const { return error_; }
    const std::string &gameId() const { return gameId_; }

    std::atomic<bool> cancel{false};

private:
    std::string error_;
    std::string gameId_;
};

} // namespace swfimport
