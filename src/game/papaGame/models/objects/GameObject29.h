#pragma once
// AS: papaGame.models.objects.GameObject29 — "Start Fight": an invisible sensor; when the player touches it, it
// switches off (its trigger closes the doors and drops the boss) and can lock the camera on the arena.
#include "game/papaGame/models/GameObject.h"

namespace papaGame::models::objects {

class GameObject29 : public GameObject {
public:
    using GameObject::GameObject;

    void defineVars() override;
    void saveObjectData() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    bool checkOnScreen() override;
    void updateObject() override;
    void finishDeactivatingObject() override;

    bool shouldLockCamera = false;
    double cameraLockX = 0;
    double cameraLockY = 0;
};

} // namespace papaGame::models::objects
