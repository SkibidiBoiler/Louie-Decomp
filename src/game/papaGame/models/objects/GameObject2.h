#pragma once
// AS: papaGame.models.objects.GameObject2 — the pushable boulder: a customer with the push skill rolls it (it speeds
// up, keeps rolling and slows down, rolls down slopes), it carries riders and squashes the enemies it rolls or
// falls into.
#include <memory>

#include "engine/audio.h"
#include "game/papaGame/models/GameObject.h"

namespace papaGame::models::objects {

class GameObject2 : public GameObject {
public:
    using GameObject::GameObject;

    void defineVars() override;
    void setupParams(const std::vector<as3::Value> & /*params*/) override {} // AS: GameObject2 (empty body)
    void saveObjectData() override;
    void destroy() override;
    const AnimationCycle &grabAnimationCycle() override;
    bool checkOnScreen() override;
    void updateObject() override;
    void getHit(double /*damage*/ = 1) override {} // AS: GameObject2.getHit (empty)
    // AS: GameObject2.checkHittingEnemies (overrides GameObject's, which the port has not needed yet)
    bool checkHittingEnemies(double unused1 = 0, double unused2 = 0);

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

private:
    void pushFromSide(bool fromLeft);
};

} // namespace papaGame::models::objects
