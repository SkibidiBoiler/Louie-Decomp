#pragma once
// AS: papaGame.models.objects.GameObject14 — the drop berry: hangs in the air; stood on, it drops after a moment
// (carrying its riders), splats where it lands and refills back where it hung.
#include "game/papaGame/models/GameObject.h"

namespace papaGame::models::objects {

class GameObject14 : public GameObject {
public:
    using GameObject::GameObject;

    void init(double xtile, double ytile, double id, double whichScreen, double screenObjID,
              const std::vector<as3::Value> &extraParams) override;
    void defineVars() override;
    void setupParams(const std::vector<as3::Value> & /*params*/) override {} // AS: GameObject14.setupParams (empty)
    void saveObjectData() override {}                                     // AS: GameObject14.saveObjectData (empty)
    void destroy() override {}                                            // AS: GameObject14.destroy (empty)
    void endAnimationCycle() override;
    const AnimationCycle &grabAnimationCycle() override;
    void resetBerry();
    void readyBerry();
    void splatBerry();
    void dropBerry();
    bool checkOnScreen() override;
    void stepOnObject(Character &c) override;
    void stopStandingOnObject(Character &c) override;
    void updateObject() override;
    void rideObject(Character &c, double offset);
    void getHit(double /*damage*/ = 1) override {} // AS: GameObject14.getHit (empty)

    AnimationCycle refillCycleFrames = cycleType2("refill", 1, 0, -1, {0, 1, 2, 3, 4, 5, 6, 7, 8});
    AnimationCycle hangCycleFrames =
        cycleType2("hang", 1, 0, 0, {9, 10, 11, 12, 13, 14, 15, 16, 15, 14, 13, 12, 11, 10});
    AnimationCycle standCycleFrames = cycleType2("stand", 1, 0, 0, {17});
    AnimationCycle fallCycleFrames = cycleType2("fall", 1, 0, 2, {18, 18, 19});
    AnimationCycle splatCycleFrames =
        cycleType2("splat", 1, 0, -1, {20, 21, 22, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23});
    std::vector<double> spriteFrameOffsets = std::vector<double>(26, 11);
    bool playerDidStep = false;
    bool isSplattering = false;
    bool isResetting = false;
    double standTimer = 0;
    double standTimerMax = 20;
    double startingYtile = 0;
    bool waitForPlayer = true;
    bool hasPlayer = false;
    bool isMoving = false;
};

} // namespace papaGame::models::objects
