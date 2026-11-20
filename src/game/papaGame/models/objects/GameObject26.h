#pragma once
// AS: papaGame.models.objects.GameObject26 — the Cheese Cannon: every shootDelay frames it fires a cheese shot at
// the player (only when it is on screen); a hit leaves the player stuck in cheese. It cannot be hurt.
#include "game/papaGame/models/GameObject.h"

namespace papaGame::models::objects {

class GameObject26 : public GameObject {
public:
    using GameObject::GameObject;

    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    void saveObjectData() override;
    void destroy() override;
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    bool checkOnScreen() override;
    void updateObject() override;
    void getHit(double /*damage*/ = 1) override {} // AS: GameObject26.getHit (empty)
    bool checkReallyOnScreen() const;
    void shootCheese();

    double mainTimer = 0;
    double shootDelay = 120;
    double startedMainTimer = 0;
    AnimationCycle staticCycleFrames = cycleType2("static", 1, 0, 0, {0});
    AnimationCycle readyCycleFrames = cycleType2("ready", 1, 0, -1, {0, 1, 2, 3, 4, 3, 4, 3, 4, 3, 4});
    AnimationCycle shootCycleFrames = cycleType2("shoot", 1, 0, -1, {2, 1, 0, 5, 6, 6, 6, 6, 5});
    bool isGettingReady = false;
    bool isShooting = false;
};

} // namespace papaGame::models::objects
