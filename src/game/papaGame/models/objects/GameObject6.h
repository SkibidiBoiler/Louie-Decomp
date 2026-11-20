#pragma once
// AS: papaGame.models.objects.GameObject6 — the horizontal blimp: shuttles between its column and a target column
// (or waits for the player to board), carrying its riders; a boss can spawn one that flies in from off screen.
#include "game/papaGame/models/GameObject.h"

namespace papaGame::models::objects {

class GameObject6 : public GameObject {
public:
    using GameObject::GameObject;

    void init(double xtile, double ytile, double id, double whichScreen, double screenObjID,
              const std::vector<as3::Value> &extraParams) override;
    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    void saveObjectData() override {} // AS: GameObject6.saveObjectData (empty)
    void destroy() override {}        // AS: GameObject6.destroy (empty)
    void endAnimationCycle() override;
    const AnimationCycle &grabAnimationCycle() override;
    bool checkOnScreen() override;
    void stepOnObject(Character &c) override;
    void stopStandingOnObject(Character &c) override;
    void updateObject() override;
    void getHit(double /*damage*/ = 1) override {} // AS: GameObject6.getHit (empty)

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
    double startingXtile = 0;
    double targetXtile = 0;
    bool waitForPlayer = false;
    bool hasPlayer = false;
    bool isMoving = false;
    bool spawnedByBoss = false;
    bool isTweeningIn = false;
};

} // namespace papaGame::models::objects
