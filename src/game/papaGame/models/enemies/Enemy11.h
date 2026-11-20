#pragma once
// AS: papaGame.models.enemies.Enemy11 — the Party Sub's front car (Enemy9 with its own sheet).
#include "game/papaGame/models/enemies/Enemy9.h"

namespace papaGame::models::enemies {

class Enemy11 : public Enemy9 {
public:
    using Enemy9::Enemy9;

    void defineVars() override;
};

} // namespace papaGame::models::enemies
