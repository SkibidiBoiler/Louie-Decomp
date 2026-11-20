#pragma once
// AS: papaGame.models.objects.GameObject18 — a door: solid (four tiles high) while shut, opened and closed by
// triggers; its front frame (DoorFrontEffect) is drawn over the player once it is open.
#include "game/papaGame/models/GameObject.h"

namespace papaGame::models::objects {

class GameObject18 : public GameObject {
public:
    using GameObject::GameObject;

    void init(double xtile, double ytile, double id, double whichScreen, double screenObjID,
              const std::vector<as3::Value> &extraParams) override;
    void defineVars() override;
    void saveObjectData() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    const AnimationCycle &grabAnimationCycle() override;
    bool checkOnScreen() override;
    void updateObject() override;
    void destroy() override;
    void finishDeactivatingObject() override;
    void finishActivatingObject() override;

    double frontEffectID = -1;
};

} // namespace papaGame::models::objects
