#include "game/papaGame/managers/ItemManager.h"

#include <SDL3/SDL.h>

#include "engine/json.h"
#include "game/obf_9G/obf_4R.h"
#include "game/papaGame/models/items/GameItem_Balloon.h"
#include "game/papaGame/models/items/GameItem_Coin.h"
#include "game/papaGame/models/items/GameItem_Daisy.h"
#include "game/papaGame/models/items/GameItem_FizzoCan.h"
#include "game/papaGame/models/items/GameItem_GoldHelmet.h"
#include "game/papaGame/models/items/GameItem_GummyWorm.h"
#include "game/papaGame/models/items/GameItem_Heart.h"
#include "game/papaGame/models/items/GameItem_PapaCoin.h"
#include "game/papaGame/models/items/GameItem_RadishCoin.h"
#include "game/papaGame/models/items/GameItem_SargeCoin.h"

namespace papaGame::managers {

namespace {

// Port: getDefinitionByName("papaGame.models.items.GameItem_" + type).
std::unique_ptr<models::GameItem> createItem(obf_9G::obf_4R *gameObj, const std::string &type)
{
    using namespace models::items;
    if (type == "Coin") {
        return std::make_unique<GameItem_Coin>(gameObj);
    }
    if (type == "PapaCoin") {
        return std::make_unique<GameItem_PapaCoin>(gameObj);
    }
    if (type == "SargeCoin") {
        return std::make_unique<GameItem_SargeCoin>(gameObj);
    }
    if (type == "RadishCoin") {
        return std::make_unique<GameItem_RadishCoin>(gameObj);
    }
    if (type == "Heart") {
        return std::make_unique<GameItem_Heart>(gameObj);
    }
    if (type == "Daisy") {
        return std::make_unique<GameItem_Daisy>(gameObj);
    }
    if (type == "Balloon") {
        return std::make_unique<GameItem_Balloon>(gameObj);
    }
    if (type == "FizzoCan") {
        return std::make_unique<GameItem_FizzoCan>(gameObj);
    }
    if (type == "GummyWorm") {
        return std::make_unique<GameItem_GummyWorm>(gameObj);
    }
    if (type == "GoldHelmet") {
        return std::make_unique<GameItem_GoldHelmet>(gameObj);
    }
    return nullptr; // SKELETON: the dropped weapons
}

} // namespace

ItemManager::ItemManager(obf_9G::obf_4R *gameObj_) : gameObj(gameObj_) {}

ItemManager::~ItemManager() = default;

bool ItemManager::loadTables(const std::string &path)
{
    size_t size = 0;
    char *raw = static_cast<char *>(SDL_LoadFile(path.c_str(), &size));
    if (!raw) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot read %s (import the game again)", path.c_str());
        return false;
    }
    engine::JsonValue json;
    const bool ok = engine::parseJson(std::string_view(raw, size), json);
    SDL_free(raw);
    if (!ok) {
        return false;
    }
    if (const engine::JsonValue *manager = json.get("ItemManager")) {
        itemIDsToNames.clear();
        if (const engine::JsonValue *list = manager->get("itemIDsToNames")) {
            for (const auto &v : list->items) {
                itemIDsToNames.push_back(v.string);
            }
        }
    }
    if (const engine::JsonValue *base = json.get("GameItem")) {
        models::GameItem::loadTables(*base);
    }
    return !itemIDsToNames.empty();
}

// AS: ItemManager.checkForCollecting
// QUIRK: an item removed while collecting shifts the list, so the item after it is not checked this frame.
void ItemManager::checkForCollecting(const models::Rect &r)
{
    for (size_t i = 0; i < items.size(); ++i) {
        models::GameItem *item = items[i];
        if (item && item->checkOnScreen() && item->canPickUp() && item->checkSpriteCollision(r)) {
            item->collectItem();
        }
    }
}

// AS: ItemManager.getCorrectItemName — an index into itemIDsToNames, or the name itself.
std::string ItemManager::getCorrectItemName(const as3::Value &type) const
{
    if (type.isNumber && type.number < static_cast<double>(itemIDsToNames.size())) {
        return type.number >= 0 ? itemIDsToNames[static_cast<size_t>(type.number)] : std::string{};
    }
    return type.isNumber ? as3::String(type.number) : type.string;
}

// AS: ItemManager.addItem — returns the new item's id.
double ItemManager::addItem(double xtile, double ytile, const as3::Value &type, double whichScreen,
                            const std::vector<as3::Value> &extraParams, bool isDropped)
{
    if (!type.isNumber && type.string.starts_with("$")) {
        // SKELETON: dropMoney(amount, xtile, ytile).
        return 0;
    }
    ++counter;
    const double objectid = counter;
    const std::string name = getCorrectItemName(type);
    std::unique_ptr<models::GameItem> item = createItem(gameObj, name);
    if (!item) {
        SDL_Log("Item doesn't exist yet!  Type=%s", name.c_str());
        return objectid;
    }
    item->init(xtile, ytile, objectid, whichScreen, extraParams, isDropped);
    models::GameItem *it = item.get();
    double sheetIndex = -1;
    for (size_t i = 0; i < objectTypes.size(); ++i) {
        if (objectTypes[i] == name) {
            sheetIndex = static_cast<double>(i);
            break;
        }
    }
    if (sheetIndex == -1) {
        objectTypes.push_back(name);
        objectSheets.push_back(gameObj->bitmapManager->sheets->get(it->sheetname));
        sheetIndex = static_cast<double>(objectSheets.size()) - 1;
    }
    it->setSheetIndex(sheetIndex);
    byID_[objectid] = std::move(item);
    items.push_back(it);
    return objectid;
}

// AS: ItemManager.processItems — last to first.
void ItemManager::processItems()
{
    graveyard_.clear();
    for (int i = static_cast<int>(items.size()) - 1; i >= 0; --i) {
        if (i < static_cast<int>(items.size()) && items[static_cast<size_t>(i)]) {
            items[static_cast<size_t>(i)]->updateObject();
        }
    }
}

// AS: ItemManager.saveItems — the items still in the room become its new itemArray.
void ItemManager::saveItems()
{
    std::vector<std::vector<as3::Value>> saved;
    for (int i = static_cast<int>(items.size()) - 1; i >= 0; --i) {
        models::GameItem *item = items[static_cast<size_t>(i)];
        if (item && !item->itemType.empty()) {
            if (item->shouldSaveOnRoomChange || (item->isWeapon && !item->isEmpty)) {
                saved.push_back(item->getItemData());
            }
        }
    }
    for (auto &[id, item] : byID_) {
        graveyard_.push_back(std::move(item));
    }
    byID_.clear();
    counter = 0;
    items.clear();
    gameObj->obf_AQ->currentScreenData->updateItems(saved);
    objectSheets.clear();
    objectTypes.clear();
}

// AS: ItemManager.saveItemsAtCheckpoint — like saveItems, but the items stay in the room.
void ItemManager::saveItemsAtCheckpoint()
{
    std::vector<std::vector<as3::Value>> saved;
    for (int i = static_cast<int>(items.size()) - 1; i >= 0; --i) {
        models::GameItem *item = items[static_cast<size_t>(i)];
        if (item && !item->itemType.empty()) {
            if (item->shouldSaveOnRoomChange || (item->isWeapon && !item->isEmpty)) {
                saved.push_back(item->getItemData());
            }
        }
    }
    gameObj->obf_AQ->currentScreenData->updateItems(saved);
}

// AS: ItemManager.clearItems
void ItemManager::clearItems(double whichScreen, bool all, bool resetCounter)
{
    for (int i = static_cast<int>(items.size()) - 1; i >= 0; --i) {
        models::GameItem *item = items[static_cast<size_t>(i)];
        if (item && (item->whichScreen == whichScreen || all)) {
            auto it = byID_.find(item->id);
            if (it != byID_.end()) {
                graveyard_.push_back(std::move(it->second));
                byID_.erase(it);
            }
            items.erase(items.begin() + i);
        }
    }
    if (resetCounter) {
        counter = static_cast<double>(items.size());
    }
    if (all) {
        counter = 0;
        items.clear();
    }
    objectSheets.clear();
    objectTypes.clear();
}

// AS: ItemManager.buildItems — rows of itemArray: [type, x, y, params...]; type 0 is a taken item.
void ItemManager::buildItems(double whichScreen)
{
    const display::GameDisplay &d = *gameObj->obf_71;
    const double offsetX = whichScreen * d.screenTileWidth;
    const auto rows = gameObj->obf_AQ->currentScreenData->itemArray;
    for (const auto &row : rows) {
        const as3::Value type = row.empty() ? as3::Value::num(0) : row[0];
        if (!(type.isNumber && type.number == 0)) {
            const double x = offsetX + (row.size() > 1 ? row[1].asNumber() : 0);
            const double y = row.size() > 2 ? row[2].asNumber() : 0;
            std::vector<as3::Value> params;
            if (row.size() > 3) {
                params.assign(row.begin() + 3, row.end());
            }
            addItem(x, y, type, whichScreen, params);
        } else {
            ++counter;
        }
    }
}

// AS: ItemManager.removeItem
void ItemManager::removeItem(double itemID, bool nosplice)
{
    for (size_t i = 0; i < items.size(); ++i) {
        if (items[i] && items[i]->id == itemID) {
            items[i]->destroy();
            auto it = byID_.find(itemID);
            if (it != byID_.end()) {
                graveyard_.push_back(std::move(it->second));
                byID_.erase(it);
            }
            items[i] = nullptr;
            if (!nosplice) {
                items.erase(items.begin() + static_cast<std::ptrdiff_t>(i));
            }
            return;
        }
    }
}

} // namespace papaGame::managers
