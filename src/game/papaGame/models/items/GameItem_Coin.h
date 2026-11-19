#pragma once
// AS: papaGame.models.items.GameItem_Coin — a coin: money and points.
#include "game/papaGame/models/GameItem.h"

namespace papaGame::models::items {

class GameItem_Coin : public GameItem {
public:
    using GameItem::GameItem;

    void defineVars() override;
    void collectItem() override;
};

} // namespace papaGame::models::items
