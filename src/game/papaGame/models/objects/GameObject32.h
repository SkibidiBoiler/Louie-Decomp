#pragma once
// AS: papaGame.models.objects.GameObject32 — Rainbow Sauce: the BBQ quicksauce pool with the rainbow sheet and a
// bottom cap.
#include "game/papaGame/models/objects/GameObject27.h"

namespace papaGame::models::objects {

class GameObject32 : public GameObject27 {
public:
    using GameObject27::GameObject27;

    void defineVars() override;
};

} // namespace papaGame::models::objects
