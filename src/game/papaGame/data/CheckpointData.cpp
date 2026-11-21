#include "game/papaGame/data/CheckpointData.h"

#include <SDL3/SDL_log.h>

#include "game/as3.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::data {

// AS: CheckpointData.CheckpointData — the objects and items still in the room are saved into its data first; the
// starting room's start point becomes the checkpoint's tile.
CheckpointData::CheckpointData(obf_9G::obf_4R *gameObj, double whichLevel_, double startingRoomIndex_,
                               const std::vector<std::unique_ptr<ScreenData>> &screens, double startingRoomXtile_,
                               double startingRoomYtile_)
    : whichLevel(whichLevel_), startingRoomIndex(startingRoomIndex_), startingRoomXtile(startingRoomXtile_),
      startingRoomYtile(startingRoomYtile_)
{
    gameObj->obf_7u->saveObjectsAtCheckpoint();
    gameObj->obf_3Z->saveItemsAtCheckpoint();
    for (const auto &screen : screens) {
        screenDataArray.push_back(duplicateScreenData(*screen));
    }
    const auto index = static_cast<size_t>(startingRoomIndex);
    if (startingRoomIndex >= 0 && index < screenDataArray.size() && screenDataArray[index]->startPoint.size() > 2) {
        screenDataArray[index]->startPoint[1] = startingRoomXtile;
        screenDataArray[index]->startPoint[2] = startingRoomYtile;
        SDL_Log("(Set startpoint to checkpoint coords)");
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Error setting the starting room's startpoint to the checkpoint coords.");
    }
    const UserData &user = *gameObj->obf_51;
    save_gotHurt = user.gotHurt;
    save_fellInWater = user.fellInWater;
    save_killsTally = user.killsTally;
    save_livesLost = user.livesLost;
    save_gameplayTimer = gameObj->obf_8o->gameplayTimer;
    const managers::ChallengeManager &cm = *gameObj->obf_94;
    challenge1_tally = cm.getChallengeTallyForCheckpoint(whichLevel, 1);
    challenge2_tally = cm.getChallengeTallyForCheckpoint(whichLevel, 2);
    challenge3_tally = cm.getChallengeTallyForCheckpoint(whichLevel, 3);
    challenge4_tally = cm.getChallengeTallyForCheckpoint(whichLevel, 4);
    challenge5_tally = cm.getChallengeTallyForCheckpoint(whichLevel, 5);
    challenge6_tally = cm.getChallengeTallyForCheckpoint(whichLevel, 6);
    money_tally = user.money;
    burgerzillas_tally = user.burgerzillas;
    specialitems_tally = user.specialitems;
    SDL_Log("New Checkpoint Data created (World %s, Checkpoint Room %s (at %s,%s), %d room states saved)",
            as3::String(whichLevel + 1).c_str(), as3::String(startingRoomIndex).c_str(),
            as3::String(startingRoomXtile).c_str(), as3::String(startingRoomYtile).c_str(),
            static_cast<int>(screens.size()));
}

// AS: CheckpointData.duplicateScreenData — a fresh room with copies of the saved arrays; its objects are built
// again.
std::unique_ptr<ScreenData> CheckpointData::duplicateScreenData(const ScreenData &source)
{
    auto copy = std::make_unique<ScreenData>(source.gameObj, nullptr, source.whichTileset, source.roomIndex);
    copy->tileArray = source.tileArray;
    copy->eventArray = source.eventArray;
    copy->doorArray = source.doorArray;
    copy->startPoint = source.startPoint;
    copy->enemyArray = source.enemyArray;
    copy->objectArray = source.objectArray;
    copy->itemArray = source.itemArray;
    copy->roomID = source.roomID;
    copy->objectsInitialized = false;
    return copy;
}

// AS: CheckpointData.destroy
void CheckpointData::destroy()
{
    screenDataArray.clear();
}

// AS: CheckpointData.getScreenDataArray — copies, so the checkpoint can be used again.
std::vector<std::unique_ptr<ScreenData>> CheckpointData::getScreenDataArray() const
{
    std::vector<std::unique_ptr<ScreenData>> out;
    for (const auto &screen : screenDataArray) {
        out.push_back(duplicateScreenData(*screen));
    }
    return out;
}

} // namespace papaGame::data
