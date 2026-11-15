#pragma once
// AS: papaGame.models.objects.GameObject19 — the cage with a trapped customer; opening it ends the level.
#include "game/papaGame/models/GameObject.h"

namespace papaGame::models::objects {

class GameObject19 : public GameObject {
public:
    using GameObject::GameObject;

    void init(double xtile, double ytile, double id, double whichScreen, double screenObjID,
              const std::vector<as3::Value> &extraParams) override;
    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    const AnimationCycle &grabAnimationCycle() override;
    bool checkOnScreen() override;
    void updateObject() override;
    void deactivateObject() override;
    void destroy() override;
    void addPortal();

    double cageCustomerIndex = 1;
    double customerActualIndex = -1;
    double frontBarsID = -1;
    double backBarsID = -1;
    double customerFXID = -1;
    double spinnerID = -1;
    double portalID = -1;
    std::string fx_subtype;
    double fx_offset = 0;
    double spinner_offset = 64;
    double playerEndX = 0;
};

} // namespace papaGame::models::objects
