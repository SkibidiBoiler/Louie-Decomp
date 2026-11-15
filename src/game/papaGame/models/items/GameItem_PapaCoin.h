#pragma once
// AS: papaGame.models.items.GameItem_PapaCoin — a Papa coin (a special item).
#include "game/papaGame/models/GameItem.h"

namespace papaGame::models::items {

class GameItem_PapaCoin : public GameItem {
public:
    using GameItem::GameItem;

    void defineVars() override;
    void collectItem() override;
};

} // namespace papaGame::models::items
