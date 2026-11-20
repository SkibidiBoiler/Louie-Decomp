#pragma once
// AS: papaGame.models.objects.GameObject33 — an invisible training sensor: when the player overlaps it (or it comes
// on screen) it opens a training bubble; once it is off screen again, a basic training counts as done and a
// skill training's bubble is taken away.
#include <string>

#include "game/papaGame/models/GameObject.h"

namespace papaGame::models::objects {

class GameObject33 : public GameObject {
public:
    using GameObject::GameObject;

    void defineVars() override;
    void saveObjectData() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    bool checkOnScreen() override;
    void updateObject() override;
    void finishDeactivatingObject() override;

    std::string trainingFlag;
    std::string key1;
    std::string key2;
    std::string key3;
    double timeout = -1;
    std::string triggerMode;
    std::string skillType;
    bool didTryToClose = false;
};

} // namespace papaGame::models::objects
