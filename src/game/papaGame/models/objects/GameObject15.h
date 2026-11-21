#pragma once
// AS: papaGame.models.objects.GameObject15 — the On Berry: a bouncy berry platform that pops up and goes down on a
// timer (up for upDuration frames, down for downDuration); it wobbles for the last 20 frames before going down.
#include "game/papaGame/models/GameObject.h"

namespace papaGame::models::objects {

class GameObject15 : public GameObject {
public:
    using GameObject::GameObject;

    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    void saveObjectData() override;
    void destroy() override;
    const AnimationCycle &grabAnimationCycle() override;
    bool checkOnScreen() override;
    void updateObject() override;
    void getHit(double /*damage*/ = 1) override {} // AS: GameObject15.getHit (empty)
    void finishDeactivatingObject() override;
    void finishActivatingObject() override;
    bool checkReallyOnScreen() const;
    bool checkStandingOnObject(Character &c) override;
    void stepOnObject(Character &c) override;

    std::vector<double> spriteFrameOffsets{11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 10,
                                           9,  11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11};
    bool isUp = false;
    bool isDropping = false;
    bool isRising = false;
    double mainTimer = 0;
    double upDuration = 30;
    double downDuration = 30;
    double startedAsUp = 0;
    double startedMainTimer = 0;
    AnimationCycle almostCycleFrames = cycleType2("almost", 1, 0, 0, {11, 12, 13, 13, 12, 11});
};

} // namespace papaGame::models::objects
