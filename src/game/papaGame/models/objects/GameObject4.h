#pragma once
// AS: papaGame.models.objects.GameObject4 — the static blimp: bobs in place and can be stood on (it sags under the
// player). A boss can drop it in from above (tweening in to its row).
#include "game/papaGame/models/GameObject.h"

namespace papaGame::models::objects {

class GameObject4 : public GameObject {
public:
    using GameObject::GameObject;

    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    void saveObjectData() override {} // AS: GameObject4.saveObjectData (empty)
    void destroy() override;
    void endAnimationCycle() override;
    const AnimationCycle &grabAnimationCycle() override;
    bool checkOnScreen() override;
    void updateObject() override;
    void getHit(double /*damage*/ = 1) override {} // AS: GameObject4.getHit (empty)

    AnimationCycle normalCycleFrames = cycleType2(
        "normal", 1, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 1, 1, 1});
    AnimationCycle startstandCycleFrames = cycleType2("startstand", 1, 0, -1, {4, 5, 6});
    AnimationCycle standCycleFrames = cycleType2(
        "stand", 1, 0, 0, {7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 9, 9, 9, 8, 8, 8});
    AnimationCycle stopstandCycleFrames = cycleType2("stopstand", 1, 0, -1, {11, 12, 13});
    std::vector<double> spriteFrameOffsets{32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32};
    bool playerIsRiding = false;
    double waveTimer = 0;
    double wavePeriod = 16;
    double waveAmplitude = 6;
    bool spawnedByBoss = false;
    bool isTweeningIn = false;
    double targetYtile = 10;
    double tweenInDelayTimer = 0;
    double tweenInDelayTimerMax = 10;
};

} // namespace papaGame::models::objects
