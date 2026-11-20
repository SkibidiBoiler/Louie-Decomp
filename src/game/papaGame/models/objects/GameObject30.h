#pragma once
// AS: papaGame.models.objects.GameObject30 — "End Fight": switched off by the boss's defeat, it unlocks the camera
// and broadcasts (opening the doors). With "spikes" set it first plays the escape scene (the boss laughs, flees and
// blimps appear) before it broadcasts.
#include "game/papaGame/models/GameObject.h"

namespace papaGame::models::objects {

class GameObject30 : public GameObject {
public:
    using GameObject::GameObject;

    void defineVars() override;
    void saveObjectData() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    bool checkOnScreen() override;
    void updateObject() override;
    void finishDeactivatingObject() override;
    void reallyDeactivateObject();
    void handleAnimation();

    bool shouldUnlockCamera = false;
    bool isSpikes = false;
    bool shouldChoreograph = false;
    double animTimer = 0;
    bool waitingForPlayer = false;
    double escapeEffectID = -1;
};

} // namespace papaGame::models::objects
