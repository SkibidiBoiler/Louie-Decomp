#pragma once
// AS: papaGame.models.enemies.Enemy11 (extends Enemy9) — the front of a party sub.
#include "game/papaGame/models/enemies/Enemy9.h"

namespace papaGame::models::enemies {

class Enemy11 : public Enemy9 {
public:
    using Enemy9::Enemy9;

    void defineVars() override;
};

} // namespace papaGame::models::enemies
