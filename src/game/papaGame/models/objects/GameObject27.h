#pragma once
// AS: papaGame.models.objects.GameObject27 — BBQ quicksauce: a tiling pool of sauce. The player in it sinks slowly
// (isInQuicksand: slow walk, small jumps); enemies that touch it are swallowed. GameObject32 (the rainbow pool) uses
// the same code with its own splashes (its `type` is 32).
#include "game/papaGame/models/GameObject.h"

namespace papaGame::models::objects {

class GameObject27 : public GameObject {
public:
    using GameObject::GameObject;

    void defineVars() override;
    bool checkOnScreen() override { return true; } // AS: GameObject27.checkOnScreen (always)
    void setupParams(const std::vector<as3::Value> &params) override;
    void updateObject() override;
    void checkEnemiesInWater();
    void checkObjectsInWater() {} // AS: GameObject27.checkObjectsInWater (builds its rectangle, then nothing)
    void destroy() override {}    // AS: GameObject27.destroy (empty)

    bool wasMovingPlayer = false;
    bool wasPlayerJumping = false;
    double playerSplashID = -1;

private:
    void splash(double x); // port: the splash both functions add, by `type`
};

} // namespace papaGame::models::objects
