#pragma once
// AS: papaGame.managers.MapManager — tile properties of the current room (collision, slopes, ladders, ...).
// Ported so far: isWithinBounds and getTileProperty (not "door", which needs WorldData.getDoorArray).
#include <string_view>

namespace obf_9G { class obf_4R; }

namespace papaGame::managers {

class MapManager {
public:
    explicit MapManager(obf_9G::obf_4R *gameObj_) : gameObj(gameObj_) {}

    bool isWithinBounds(double x, double y) const;
    // The original returns `false` or a Number; here false is 0.
    double getTileProperty(double x, double y, std::string_view property) const;

    obf_9G::obf_4R *gameObj = nullptr;
};

} // namespace papaGame::managers
