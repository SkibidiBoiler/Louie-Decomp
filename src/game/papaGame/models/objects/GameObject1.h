#pragma once
// AS: papaGame.models.objects.GameObject1 — the cage pulley: the player grabs its handle and rides it down, which
// opens the cage (through the room's event).
#include "game/papaGame/models/GameObject.h"

namespace papaGame::models::objects {

class GameObject1 : public GameObject {
public:
    using GameObject::GameObject;

    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    const AnimationCycle &grabAnimationCycle() override;
    bool checkOnScreen() override;
    void updateObject() override;
    void grabRideableObject(Character *c) override;
    void releaseRideableObject() override;
    void getHit(double /*damage*/ = 1) override {}              // AS: GameObject1.getHit (empty)
    double getRideableDirection() const override { return 0; } // AS: GameObject1.getRideableDirection
    void saveObjectData() override;

    Character *grabbingChar = nullptr;
};

} // namespace papaGame::models::objects
