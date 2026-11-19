#pragma once
// AS: papaGame.models.items.GameItem_Heart — a heart: one health back; it stays, empty, once taken.
#include "game/papaGame/models/GameItem.h"

namespace papaGame::models::items {

class GameItem_Heart : public GameItem {
public:
    using GameItem::GameItem;

    void defineVars() override;
    void collectItem() override;
    const AnimationCycle &grabAnimationCycle() override;

    bool wasCollected = false;
    AnimationCycle collectedCycleFrames = cycleType2("collected", 1, 0, 0, {99});
};

} // namespace papaGame::models::items
