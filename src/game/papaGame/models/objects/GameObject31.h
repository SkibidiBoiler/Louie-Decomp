#pragma once
// AS: papaGame.models.objects.GameObject31 — the Spike Bed: a wide row of spikes that, once triggered, shakes and
// then rises 2 px a frame up to its target row; touching it kills the player (and any enemy) at once.
#include "game/papaGame/models/GameObject.h"

namespace papaGame::models::objects {

class GameObject31 : public GameObject {
public:
    using GameObject::GameObject;

    void defineVars() override;
    bool checkOnScreen() override { return true; } // AS: GameObject31.checkOnScreen (always)
    void setupParams(const std::vector<as3::Value> &params) override;
    void updateObject() override;
    void checkSpikingEnemies();
    void destroy() override {} // AS: GameObject31.destroy (empty)

    double targetYtile = 0;
    double riseSpeed = 2;
    double shakeTimer = 0;
    double shakeTimerMax = 30;
    bool isDone = false;
    double delayTimer = 0;
    double delayTimerMax = 16;
};

} // namespace papaGame::models::objects
