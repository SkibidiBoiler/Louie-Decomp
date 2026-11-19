#pragma once
// AS: papaGame.models.objects.GameObject22 — a 1x2 cracker block (GameObject21 with another size).
#include "game/papaGame/models/objects/GameObject21.h"

namespace papaGame::models::objects {

class GameObject22 : public GameObject21 {
public:
    using GameObject21::GameObject21;

    void defineVars() override;
};

} // namespace papaGame::models::objects
