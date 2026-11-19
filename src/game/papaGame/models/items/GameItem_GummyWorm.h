#pragma once
// AS: papaGame.models.items.GameItem_GummyWorm — a gummy worm that crawls along the ground (a special item).
#include "game/papaGame/models/GameItem.h"

namespace papaGame::models::items {

class GameItem_GummyWorm : public GameItem {
public:
    using GameItem::GameItem;

    void defineVars() override;
    void collectItem() override;
    void updateObject() override;
};

} // namespace papaGame::models::items
