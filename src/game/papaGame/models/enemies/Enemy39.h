#pragma once
// AS: papaGame.models.enemies.Enemy39 — Radley Madish's mustard rocket: the BBQ rocket that can be hit back at him.
#include "game/papaGame/models/enemies/Enemy38.h"

namespace papaGame::models::enemies {

class Enemy39 : public Enemy38 {
public:
    using Enemy38::Enemy38;

    void defineVars() override;
};

} // namespace papaGame::models::enemies
