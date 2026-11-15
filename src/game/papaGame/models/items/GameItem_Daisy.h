#pragma once
// AS: papaGame.models.items.GameItem_Daisy — a daisy (a special item).
#include "game/papaGame/models/GameItem.h"

namespace papaGame::models::items {

class GameItem_Daisy : public GameItem {
public:
    using GameItem::GameItem;

    void defineVars() override;
    void collectItem() override;
};

} // namespace papaGame::models::items
