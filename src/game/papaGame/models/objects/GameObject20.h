#pragma once
// AS: papaGame.models.objects.GameObject20 — the cage facing the other way (GameObject19 flipped).
#include "game/papaGame/models/objects/GameObject19.h"

namespace papaGame::models::objects {

class GameObject20 : public GameObject19 {
public:
    using GameObject19::GameObject19;

    void init(double xtile, double ytile, double id, double whichScreen, double screenObjID,
              const std::vector<as3::Value> &extraParams) override;
    void defineVars() override;
};

} // namespace papaGame::models::objects
