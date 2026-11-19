#pragma once
// AS: papaGame.models.objects.GameObject23 — a 2x1 cracker block (GameObject21 with another size).
#include "game/papaGame/models/objects/GameObject21.h"

namespace papaGame::models::objects {

class GameObject23 : public GameObject21 {
public:
    using GameObject21::GameObject21;

    void defineVars() override;
};

} // namespace papaGame::models::objects
