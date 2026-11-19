#pragma once
// AS: papaGame.models.objects.GameObject28 — a checkpoint: touching it saves the checkpoint and refills health.
#include "game/papaGame/models/GameObject.h"

namespace papaGame::models::objects {

class GameObject28 : public GameObject {
public:
    using GameObject::GameObject;

    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    void updateObject() override;
    void finishDeactivatingObject() override;
    void saveObjectData() override;

    double checkpointID = 1;
    double effectID = -1;
};

} // namespace papaGame::models::objects
