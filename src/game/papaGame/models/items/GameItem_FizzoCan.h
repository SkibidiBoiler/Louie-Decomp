#pragma once
// AS: papaGame.models.items.GameItem_FizzoCan — a Fizzo can (a special item).
#include "game/papaGame/models/GameItem.h"

namespace papaGame::models::items {

class GameItem_FizzoCan : public GameItem {
public:
    using GameItem::GameItem;

    void defineVars() override;
    void collectItem() override;
};

} // namespace papaGame::models::items
