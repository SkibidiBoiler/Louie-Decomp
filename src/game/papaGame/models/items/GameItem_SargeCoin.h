#pragma once
// AS: papaGame.models.items.GameItem_SargeCoin — a Sarge coin (a special item).
#include "game/papaGame/models/GameItem.h"

namespace papaGame::models::items {

class GameItem_SargeCoin : public GameItem {
public:
    using GameItem::GameItem;

    void defineVars() override;
    void collectItem() override;
};

} // namespace papaGame::models::items
