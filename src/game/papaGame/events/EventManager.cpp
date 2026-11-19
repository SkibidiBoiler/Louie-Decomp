#include "game/papaGame/events/EventManager.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::events {

// AS: EventManager.addEvent — each "from" member gets the "to" list; each "to" member needs every "from".
void EventManager::addEvent(const std::vector<std::vector<double>> &from, const std::vector<std::vector<double>> &to,
                            double pan, double panTarget, double whichScreen)
{
    const double necessary = static_cast<double>(from.size());
    for (const auto &f : from) {
        if (f.size() < 2) {
            continue;
        }
        if (f[1] == 1) {
            gameObj->obf_7u->setEvent(whichScreen, f[0], to, pan, panTarget);
        } else if (f[1] == 2) {
            gameObj->obf_E->setEvent(whichScreen, f[0], to, pan, panTarget);
        }
    }
    for (const auto &t : to) {
        if (t.size() < 2) {
            continue;
        }
        if (t[1] == 1) {
            gameObj->obf_7u->setEventTriggers(whichScreen, t[0], necessary, from);
        } else if (t[1] == 2) {
            gameObj->obf_E->setEventTriggers(whichScreen, t[0], necessary, from);
        }
    }
}

// AS: EventManager.loadEvents — eventArray rows: [from, to, pan, panTarget (default 1)].
void EventManager::loadEvents(double whichScreen)
{
    for (const data::ScreenData::Event &ev : gameObj->obf_AQ->currentScreenData->eventArray) {
        const double panTarget = ev.fieldCount > 3 ? ev.field3 : 1;
        addEvent(ev.field0, ev.field1, ev.field2, panTarget, whichScreen);
    }
}

} // namespace papaGame::events
