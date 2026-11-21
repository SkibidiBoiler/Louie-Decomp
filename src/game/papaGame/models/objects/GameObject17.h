#pragma once
// AS: papaGame.models.objects.GameObject17 — a floor switch: stepping on it presses it down (deactivates it), which
// broadcasts its trigger; objects can press it too.
#include "game/papaGame/models/GameObject.h"

namespace papaGame::models::objects {

class GameObject17 : public GameObject {
public:
    using GameObject::GameObject;

    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    void saveObjectData() override;
    void destroy() override;
    const AnimationCycle &grabAnimationCycle() override;
    bool checkOnScreen() override;
    void updateObject() override;
    void getHit(double /*damage*/ = 1) override {} // AS: GameObject17.getHit (empty)
    void deactivateObject() override;
    void stepOnObject(Character &c) override;

    std::vector<double> spriteFrameOffsets{0, 3, 6, 9, 12, 15, 18, 21, 26, 32};
};

} // namespace papaGame::models::objects
