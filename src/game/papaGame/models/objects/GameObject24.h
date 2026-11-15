#pragma once
// AS: papaGame.models.objects.GameObject24 — a 2x2 cracker block (GameObject21 with another size).
#include "game/papaGame/models/objects/GameObject21.h"

namespace papaGame::models::objects {

class GameObject24 : public GameObject21 {
public:
    using GameObject21::GameObject21;

    void defineVars() override;
};

} // namespace papaGame::models::objects
