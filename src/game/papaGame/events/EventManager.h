#pragma once
// AS: papaGame.events.EventManager (§_-7t§ on the game object) — wires the room's eventArray: which objects and
// enemies (the "from" list) tell which others (the "to" list) when they are triggered.
#include <vector>

namespace obf_9G { class obf_4R; }

namespace papaGame::events {

class EventManager {
public:
    explicit EventManager(obf_9G::obf_4R *gameObj_) : gameObj(gameObj_) {}

    // `from` / `to`: [screen object id, 1 = object | 2 = enemy] pairs.
    void addEvent(const std::vector<std::vector<double>> &from, const std::vector<std::vector<double>> &to,
                  double pan, double panTarget, double whichScreen);
    void loadEvents(double whichScreen);

    obf_9G::obf_4R *gameObj = nullptr;
};

} // namespace papaGame::events
