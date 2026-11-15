#pragma once
// AS: papaGame.models.objects.GameObject3 — a ladder: marks its tiles as "ladder" and is drawn by blitLadder.
#include "game/papaGame/models/GameObject.h"

namespace papaGame::models::objects {

class GameObject3 : public GameObject {
public:
    using GameObject::GameObject;

    void init(double xtile, double ytile, double id, double whichScreen, double screenObjID,
              const std::vector<as3::Value> &extraParams) override;
    void defineVars() override;
    bool checkOnScreen() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    void updateObject() override;
};

} // namespace papaGame::models::objects
