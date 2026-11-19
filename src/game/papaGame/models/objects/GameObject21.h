#pragma once
// AS: papaGame.models.objects.GameObject21 — a 1x1 cracker block: solid until broken (by a ground pound, or by
// explosives), remembered once broken.
#include "game/papaGame/models/GameObject.h"

namespace papaGame::models::objects {

class GameObject21 : public GameObject {
public:
    using GameObject::GameObject;

    void defineVars() override;
    void saveObjectData() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    const AnimationCycle &grabAnimationCycle() override;
    void getHit(double damage = 1) override;
    void addBreakEffects();
    bool checkOnScreen() override;
    void updateObject() override;

    double hitsToBreak = 1;
    bool rememberBroken = true;
};

} // namespace papaGame::models::objects
