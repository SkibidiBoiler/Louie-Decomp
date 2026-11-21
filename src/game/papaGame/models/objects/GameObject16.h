#pragma once
// AS: papaGame.models.objects.GameObject16 — the Off Berry: the On Berry with the orange sheet and sound.
#include "game/papaGame/models/objects/GameObject15.h"

namespace papaGame::models::objects {

class GameObject16 : public GameObject15 {
public:
    using GameObject15::GameObject15;

    void defineVars() override;
};

} // namespace papaGame::models::objects
