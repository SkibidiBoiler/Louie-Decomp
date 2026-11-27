#pragma once
// AS: papaGame.managers.ItemManager (§_-3Z§ on the game object) — builds, runs, collects and saves the room's
// items. SKELETON: money drops (dropMoney names PL3 items that PL2 lacks), dropped enemy weapons, checkpoints.
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "engine/sheets.h"
#include "game/as3.h"
#include "game/papaGame/models/GameItem.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::managers {

class ItemManager {
public:
    explicit ItemManager(obf_9G::obf_4R *gameObj);
    ~ItemManager();

    // Port: ItemManager.itemIDsToNames and the base GameItem's cycles, from items.json.
    bool loadTables(const std::string &path);

    void checkForCollecting(const models::Rect &r);
    std::string getCorrectItemName(const as3::Value &type) const;
    double addItem(double xtile, double ytile, const as3::Value &type, double whichScreen,
                   const std::vector<as3::Value> &extraParams = {}, bool isDropped = false);
    void processItems();
    void saveItems();
    void saveItemsAtCheckpoint();
    void clearItems(double whichScreen, bool all = false, bool resetCounter = true);
    void destroy() { clearItems(1, true); } // AS: ItemManager.destroy
    void buildItems(double whichScreen);
    void removeItem(double itemID, bool nosplice = false);

    obf_9G::obf_4R *gameObj = nullptr;
    double counter = 0;
    std::vector<std::string> itemIDsToNames;
    std::vector<models::GameItem *> items;
    std::vector<std::string> objectTypes;
    std::vector<const engine::Sheet *> objectSheets;

private:
    std::map<double, std::unique_ptr<models::GameItem>> byID_;
    std::vector<std::unique_ptr<models::GameItem>> graveyard_; // removed items wait a frame (see EnemyManager)
};

} // namespace papaGame::managers
