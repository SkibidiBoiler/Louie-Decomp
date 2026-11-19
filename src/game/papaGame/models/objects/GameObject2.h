#pragma once
// AS: papaGame.models.objects.GameObject2 — the pushable boulder: a pushing customer rolls it (speeding up, slowing
// down on its own, rolling down slopes); it can be stood on, and it flattens the enemies it rolls or falls onto.
#include <memory>

#include "game/papaGame/models/GameObject.h"

namespace engine { class SoundChannel; }

namespace papaGame::models::objects {

class GameObject2 : public GameObject {
public:
    using GameObject::GameObject;

    void defineVars() override;
    void setupParams(const std::vector<as3::Value> & /*params*/) override {} // AS: GameObject2.setupParams (empty)
    void saveObjectData() override;
    void destroy() override;
    const AnimationCycle &grabAnimationCycle() override;
    bool checkOnScreen() override;
    void updateObject() override;
    void getHit(double /*damage*/ = 1) override {} // AS: GameObject2.getHit (empty)
    bool checkHittingEnemies();

    AnimationCycle movingCycleFrames = cycleType2("moving", 1, 0, 0, {0});
    std::vector<double> spriteFrameOffsets = std::vector<double>(33, 24);
    double rollingSpeed = 0;
    double startingSpeed = 1;
    double maxSpeed = 6;
    double speedAcceleration = 6;
    double speedDeceleration = 12;
    double speedTimer = 0;
    double maxRidingSpeed = 12;
    double ridingFastDeceleration = 10;
    double ridingDeceleration = 60;
    bool wasNudged = false;
    double nudgeDirection = 1;
    double playerLastX = 0;
    std::shared_ptr<engine::SoundChannel> pushSound;
    bool didFlagFinishTraining = false;
};

} // namespace papaGame::models::objects
