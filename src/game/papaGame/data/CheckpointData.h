#pragma once
// AS: papaGame.data.CheckpointData — what a checkpoint keeps: a copy of every room's state, the room and tile to
// start from, and the level's tallies (hurt / water flags, kills, lives lost, time, money, Burgerzillas, special
// items, the counted challenges).
#include <memory>
#include <vector>

#include "game/papaGame/data/ScreenData.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::data {

class CheckpointData {
public:
    CheckpointData(obf_9G::obf_4R *gameObj, double whichLevel, double startingRoomIndex,
                   const std::vector<std::unique_ptr<ScreenData>> &screenDataArray, double startingRoomXtile = -1,
                   double startingRoomYtile = -1);

    static std::unique_ptr<ScreenData> duplicateScreenData(const ScreenData &source);
    void destroy();
    std::vector<std::unique_ptr<ScreenData>> getScreenDataArray() const;
    double getStartingRoomIndex() const { return startingRoomIndex; } // AS: CheckpointData.getStartingRoomIndex
    double getCheckpointLevel() const { return whichLevel; }          // AS: CheckpointData.getCheckpointLevel

    double whichLevel = -1;
    double startingRoomIndex = -1;
    std::vector<std::unique_ptr<ScreenData>> screenDataArray;
    double startingRoomXtile = -1;
    double startingRoomYtile = -1;
    bool save_gotHurt = false;
    bool save_fellInWater = false;
    double save_killsTally = 0;
    double save_livesLost = 0;
    double save_gameplayTimer = 0;
    double challenge1_tally = -1;
    double challenge2_tally = -1;
    double challenge3_tally = -1;
    double challenge4_tally = -1;
    double challenge5_tally = -1;
    double challenge6_tally = -1;
    double money_tally = -1;
    double burgerzillas_tally = -1;
    double specialitems_tally = -1;
};

} // namespace papaGame::data
