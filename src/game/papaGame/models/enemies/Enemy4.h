#pragma once
// AS: papaGame.models.enemies.Enemy4 (extends Enemy3) — the army onion with a gold helmet, which it drops.
#include "game/papaGame/models/enemies/Enemy3.h"

namespace papaGame::models::enemies {

class Enemy4 : public Enemy3 {
public:
    using Enemy3::Enemy3;

    void defineVars() override;
    void killMe(double dir = 1) override;
};

} // namespace papaGame::models::enemies
