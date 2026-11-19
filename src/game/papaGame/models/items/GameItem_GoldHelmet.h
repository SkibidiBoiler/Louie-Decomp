#pragma once
// AS: papaGame.models.items.GameItem_GoldHelmet — the gold helmet a gold army onion drops: it pops up and bounces
// (a special item).
#include "game/papaGame/models/GameItem.h"

namespace papaGame::models::items {

class GameItem_GoldHelmet : public GameItem {
public:
    using GameItem::GameItem;

    void init(double xtile, double ytile, double id, double whichScreen, const std::vector<as3::Value> &extraParams,
              bool isDropped) override;
    void defineVars() override;
    void collectItem() override;
    void updateObject() override;

    double whichBounceSound = 0;
};

} // namespace papaGame::models::items
