#pragma once
// AS: papaGame.models.items.GameItem_Balloon — a balloon that bobs up and down (a special item).
#include "game/papaGame/models/GameItem.h"

namespace papaGame::models::items {

class GameItem_Balloon : public GameItem {
public:
    using GameItem::GameItem;

    void init(double xtile, double ytile, double id, double whichScreen,
              const std::vector<as3::Value> &extraParams, bool isDropped) override;
    void defineVars() override;
    void collectItem() override;
    void updateObject() override;

    double initialY = 0;
    double waveTimer = 0;
    double wavePeriod = 16;
    double waveAmplitude = 6;
};

} // namespace papaGame::models::items
