#include "game/papaGame/data/DataManager.h"

#include <SDL3/SDL.h>

#include "game/as3.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::data {

// AS: DataManager.DataManager
DataManager::DataManager(obf_9G::obf_4R *gameObj_) : gameObj(gameObj_)
{
    setupKeyNames();
}

// AS: DataManager.setupKeyNames — the names of the Flash key codes (letters and digits are themselves).
void DataManager::setupKeyNames()
{
    keyNameMap.assign(223, "");
    for (int code = 65; code <= 90; ++code) {
        keyNameMap[static_cast<size_t>(code)] = std::string(1, static_cast<char>(code)); // A..Z
    }
    for (int code = 48; code <= 57; ++code) {
        keyNameMap[static_cast<size_t>(code)] = std::string(1, static_cast<char>(code)); // 0..9
    }
    static const std::pair<int, const char *> names[] = {
        {96, "Num 0"}, {97, "Num 1"}, {98, "Num 2"},   {99, "Num 3"},  {100, "Num 4"}, {101, "Num 5"},
        {102, "Num 6"}, {103, "Num 7"}, {104, "Num 8"}, {105, "Num 9"}, {106, "Num *"}, {107, "Num +"},
        {13, "Enter"},  {109, "Num -"}, {110, "Num ."}, {111, "Num /"}, {9, "Tab"},     {8, "Bksp"},
        {19, "Pause"},  {16, "Shift"},  {17, "Ctrl"},   {32, "Space"},  {33, "PgUp"},   {34, "PgDn"},
        {35, "End"},    {36, "Home"},   {37, "Left"},   {38, "Up"},     {39, "Right"},  {40, "Down"},
        {45, "Ins"},    {46, "Del"},    {186, ";"},     {187, "+="},    {189, "-"},     {191, "/?"},
        {192, "~"},     {219, "["},     {221, "]"},     {220, "|"},     {222, "'"},     {188, ","},
        {190, "."},
    };
    for (const auto &[code, name] : names) {
        keyNameMap[static_cast<size_t>(code)] = name;
    }
}

// AS: DataManager.getKeyLabel — the name of the key bound to an action ("???" when it has none).
std::string DataManager::getKeyLabel(const std::string &key) const
{
    const UserData &user = *gameObj->obf_51;
    std::string label = "???";
    double code = 0;
    if (key == KEY_ATTACK) {
        code = user.keyCodeAttack;
    } else if (key == KEY_JUMP) {
        code = user.keyCodeJump;
    } else if (key == KEY_DROP) {
        code = user.keyCodeDrop;
    } else if (key == KEY_RIGHT) {
        code = user.keyCodeRight;
    } else if (key == KEY_LEFT) {
        code = user.keyCodeLeft;
    } else if (key == KEY_UP) {
        code = user.keyCodeUp;
    } else if (key == KEY_DOWN) {
        code = user.keyCodeDown;
    } else if (key == KEY_PAUSE) {
        code = user.keyCodePause;
    }
    if (code >= 0 && static_cast<double>(keyNameMap.size()) > code &&
        !keyNameMap[static_cast<size_t>(code)].empty()) {
        label = keyNameMap[static_cast<size_t>(code)];
    }
    return label;
}

// AS: DataManager.prepareLevelData (reads the converted levels.xml; the original decoded it from a bitmap)
bool DataManager::prepareLevelData(const std::string &levelsXmlPath)
{
    const bool ok = levelDataClass.load(levelsXmlPath);
    levelDataLoaded(ok);
    return ok;
}

// AS: DataManager.levelDataLoaded
void DataManager::levelDataLoaded(bool loaded)
{
    if (loaded) {
        masterLevelData = levelDataClass.getLevelXMLList();
        totalLevels = static_cast<double>(masterLevelData.size());
        worldData = levelDataClass.getWorldXMLList();
        totalWorlds = static_cast<double>(worldData.size());
    }
}

// worldData.(tileset == String(n))[0]
const engine::XmlNode *DataManager::findWorld(double tileset) const
{
    const std::string wanted = as3::String(tileset);
    for (const engine::XmlNode *world : worldData) {
        if (world->childText("tileset") == wanted) {
            return world;
        }
    }
    return nullptr;
}

// AS: DataManager.setupLevelScreens
void DataManager::setupLevelScreens(double whichLevel)
{
    SDL_Log(">>>>> Setup Level Screens: %s", as3::String(whichLevel).c_str());
    currentTileset = whichLevel;
    const engine::XmlNode *worldXML = findWorld(whichLevel);
    if (worldXML == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "WORLD DOESN'T EXIST YET! Fixer -- Default to World 2.");
        whichLevel = 1;
        worldXML = findWorld(whichLevel);
        if (worldXML == nullptr) {
            return; // port: no levels.xml loaded (the original would throw here)
        }
    }
    clearLevelScreens();
    const double whichTileset = whichLevel;
    currentLevel = whichLevel;
    currentWorldData = std::make_unique<WorldData>(gameObj, *worldXML);
    if (checkpointData != nullptr && checkpointData->getCheckpointLevel() == whichLevel) {
        SDL_Log(">>>>>  Use Checkpoint Data");
        currentScreen = checkpointData->getStartingRoomIndex();
        std::vector<std::unique_ptr<ScreenData>> savedArray = checkpointData->getScreenDataArray();
        for (size_t i = 0; i < savedArray.size(); ++i) {
            if (static_cast<double>(i) == currentScreen) {
                currentScreenData = savedArray[i].get();
            }
            currentScreens.push_back(std::move(savedArray[i]));
        }
        setupCollisionArrays();
        return;
    }
    SDL_Log(">>>>> Normal Loading");
    currentScreen = currentWorldData->worldStartRoom;
    const std::vector<double> roomArray = currentWorldData->getWorldRoomIDs();
    for (size_t i = 0; i < roomArray.size(); ++i) {
        const std::string id = as3::String(roomArray[i]);
        const engine::XmlNode *useXML = nullptr;
        for (const engine::XmlNode *level : masterLevelData) {
            if (level->childText("roomID") == id) {
                useXML = level;
                break;
            }
        }
        auto thisScreen = std::make_unique<ScreenData>(gameObj, useXML, whichTileset, static_cast<double>(i));
        if (static_cast<double>(i) == currentScreen) {
            currentScreenData = thisScreen.get();
        }
        currentScreens.push_back(std::move(thisScreen));
    }
    setupCollisionArrays();
}

// AS: DataManager.getWorldTitle
std::string DataManager::getWorldTitle(double whichWorld) const
{
    const engine::XmlNode *worldXML = findWorld(whichWorld);
    return worldXML ? worldXML->childText("worldTitle") : "Doesn't Exist Yet!";
}

// AS: DataManager.setActiveRoom
void DataManager::setActiveRoom(double index)
{
    if (currentScreenData) {
        currentScreenData->objectsInitialized = false;
        // clearBitmaps(): the port keeps no per-room tile bitmap (GameDisplay draws the tiles directly).
    }
    const auto i = static_cast<size_t>(index);
    currentScreenData = i < currentScreens.size() ? currentScreens[i].get() : nullptr;
    currentScreen = index;
}

// AS: DataManager.initializeScreenObjects (SKELETON: objects, items, events and the other preblits)
void DataManager::initializeScreenObjects(double whichScreen)
{
    if (!currentScreenData->objectsInitialized) {
        gameObj->obf_7u->clearObjects(whichScreen, true);
        gameObj->obf_E->clearEnemies(whichScreen, true);
        gameObj->obf_3Z->clearItems(whichScreen, true);
        gameObj->obf_7u->buildObjects(whichScreen);
        gameObj->obf_E->buildEnemies(whichScreen);
        gameObj->obf_3Z->buildItems(whichScreen);
        gameObj->obf_7t->loadEvents(whichScreen);
        gameObj->obf_4l->preblitSheets();
        gameObj->obf_E->preblitSheets();
        gameObj->obf_7u->preblitSheets();
    } else {
        SDL_Log("ERROR: Screen %s is already initialized!", as3::String(whichScreen).c_str());
    }
}

// AS: DataManager.saveCheckpoint — replaces the level's checkpoint with one at this tile of the current room.
void DataManager::saveCheckpoint(double tileX, double tileY)
{
    SDL_Log("Saving Checkpoint...");
    checkpointData.reset();
    checkpointData = std::make_unique<CheckpointData>(gameObj, currentLevel, currentScreen, currentScreens, tileX, tileY);
}

// AS: DataManager.clearCheckpoint
void DataManager::clearCheckpoint()
{
    SDL_Log("Clearing Checkpoint");
    checkpointData.reset();
}

// AS: DataManager.handleCheckpointProgress — restarting from this level's checkpoint brings back the time and the
// tallies it saved.
void DataManager::handleCheckpointProgress()
{
    if (checkpointData != nullptr && checkpointData->whichLevel == currentLevel) {
        const CheckpointData &c = *checkpointData;
        SDL_Log("Restore progress from checkpoint");
        gameObj->obf_8o->gameplayTimer = c.save_gameplayTimer;
        UserData &user = *gameObj->obf_51;
        user.gotHurt = c.save_gotHurt;
        user.fellInWater = c.save_fellInWater;
        user.killsTally = c.save_killsTally;
        user.livesLost = c.save_livesLost;
        user.burgerzillas = c.burgerzillas_tally;
        user.money = c.money_tally;
        user.specialitems = c.specialitems_tally;
        managers::ChallengeManager &cm = *gameObj->obf_94;
        cm.setChallengeTallyFromCheckpoint(c.whichLevel, 1, c.challenge1_tally);
        cm.setChallengeTallyFromCheckpoint(c.whichLevel, 2, c.challenge2_tally);
        cm.setChallengeTallyFromCheckpoint(c.whichLevel, 3, c.challenge3_tally);
        cm.setChallengeTallyFromCheckpoint(c.whichLevel, 4, c.challenge4_tally);
        cm.setChallengeTallyFromCheckpoint(c.whichLevel, 5, c.challenge5_tally);
        cm.setChallengeTallyFromCheckpoint(c.whichLevel, 6, c.challenge6_tally);
    }
}

// AS: DataManager.clearLevelScreens
void DataManager::clearLevelScreens()
{
    currentScreenData = nullptr;
    currentWorldData.reset();
    currentScreens.clear();
}

// AS: DataManager.isOnTrain
bool DataManager::isOnTrain() const
{
    return false;
}

// AS: DataManager.isAnimatedBackground
bool DataManager::isAnimatedBackground() const
{
    return currentLevel == 9;
}

bool DataManager::loadTables(const std::string &path)
{
    size_t size = 0;
    char *raw = static_cast<char *>(SDL_LoadFile(path.c_str(), &size));
    if (!raw) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot read %s (import the game again)", path.c_str());
        return false;
    }
    const bool ok = engine::parseJson(std::string_view(raw, size), tables_);
    SDL_free(raw);
    if (!ok) {
        return false;
    }
    auto grid = [&](const char *name, Grid &out) {
        out.clear();
        if (const engine::JsonValue *v = tables_.get(name)) {
            for (const auto &row : v->items) {
                std::vector<double> r;
                for (const auto &n : row.items) {
                    r.push_back(n.number);
                }
                out.push_back(std::move(r));
            }
        }
    };
    grid("standardCollisionArray", standardCollisionArray);
    grid("standardThruArray", standardThruArray);
    grid("standardGrabArray", standardGrabArray);
    grid("outfitPrices", outfitPrices);
    coinsToUnlockWorld.clear();
    if (const engine::JsonValue *v = tables_.get("coinsToUnlockWorld")) {
        for (const auto &n : v->items) {
            coinsToUnlockWorld.push_back(n.number);
        }
    }
    enemyKey.clear();
    if (const engine::JsonValue *v = tables_.get("enemyKey")) {
        for (const auto &e : v->items) {
            EnemyKeyEntry k;
            if (const engine::JsonValue *f = e.get("id")) {
                k.id = f->number;
            }
            if (const engine::JsonValue *f = e.get("title")) {
                k.title = f->string;
            }
            if (const engine::JsonValue *f = e.get("clip")) {
                k.clip = f->string;
            }
            enemyKey.push_back(k);
        }
    }
    auto number = [&](const char *name) {
        const engine::JsonValue *v = tables_.get(name);
        return v ? v->number : 0.0;
    };
    COLLISION_TILE_NUMBER = number("COLLISION_TILE_NUMBER");
    THRUBLOCK_TILE_NUMBER = number("THRUBLOCK_TILE_NUMBER");
    GRAB_TILE_NUMBER = number("GRAB_TILE_NUMBER");
    LADDER_TILE_NUMBER = number("LADDER_TILE_NUMBER");
    SLOPE_TILE_NUMBER = number("SLOPE_TILE_NUMBER");
    lists_.clear();
    for (const auto &[name, value] : tables_.fields) {
        if (value.type == engine::JsonValue::Type::Array && (name.find("TileList") != std::string::npos)) {
            auto &set = lists_[name];
            for (const auto &n : value.items) {
                set.insert(n.number);
            }
        }
    }
    return true;
}

bool DataManager::inList(const char *list, double tile) const
{
    auto it = lists_.find(list);
    return it != lists_.end() && it->second.count(tile) > 0;
}

// AS: DataManager.setupCollisionArrays
void DataManager::setupCollisionArrays()
{
    collisionArray = &standardCollisionArray;
    thruArray = &standardThruArray;
    grabArray = &standardGrabArray;
}

// AS: DataManager.getPropValue (cells are always arrays here; the "NOT ARRAY" branch is never taken)
double DataManager::getPropValue(std::string_view property, const ScreenData::TileCell &cell) const
{
    struct Slope {
        const char *list;
        double value;
    };
    static const Slope floorSlopes[] = {
        {"slopeTileList_22A", 3}, {"slopeTileList_22B", 4}, {"slopeTileList_22C", 5}, {"slopeTileList_22D", 6},
        {"slopeTileList_45A", 1}, {"slopeTileList_45B", 2}, {"slopeTileList_67A", 7}, {"slopeTileList_67B", 8},
        {"slopeTileList_67C", 9}, {"slopeTileList_67D", 10}};
    static const Slope ceilingSlopes[] = {
        {"slopeTileList_22E", 3}, {"slopeTileList_22F", 4}, {"slopeTileList_22G", 5}, {"slopeTileList_22H", 6},
        {"slopeTileList_45C", 1}, {"slopeTileList_45D", 2}, {"slopeTileList_67E", 7}, {"slopeTileList_67F", 8},
        {"slopeTileList_67G", 9}, {"slopeTileList_67H", 10}};
    // ceilingslopereverse: the ceiling lists, then the floor lists mirrored.
    static const Slope reverseSlopes[] = {
        {"slopeTileList_22E", 3}, {"slopeTileList_22F", 4}, {"slopeTileList_22G", 5}, {"slopeTileList_22H", 6},
        {"slopeTileList_45C", 1}, {"slopeTileList_45D", 2}, {"slopeTileList_67E", 7}, {"slopeTileList_67F", 8},
        {"slopeTileList_67G", 9}, {"slopeTileList_67H", 10}, {"slopeTileList_22A", 5}, {"slopeTileList_22B", 6},
        {"slopeTileList_22C", 3}, {"slopeTileList_22D", 4}, {"slopeTileList_45A", 2}, {"slopeTileList_45B", 1},
        {"slopeTileList_67A", 9}, {"slopeTileList_67B", 10}, {"slopeTileList_67C", 7}, {"slopeTileList_67D", 8}};
    auto firstMatch = [&](const Slope *table, size_t n, double tile) {
        for (size_t i = 0; i < n; ++i) {
            if (inList(table[i].list, tile)) {
                return table[i].value;
            }
        }
        return 0.0;
    };
    double value = 0;
    for (const double tile : cell) {
        if (tile == COLLISION_TILE_NUMBER) {
            value = property == "collision" ? 1 : 0;
        } else if (tile == THRUBLOCK_TILE_NUMBER) {
            value = property == "thrublock" || property == "thru" ? 1 : 0;
        } else if (tile == GRAB_TILE_NUMBER) {
            value = property == "grab" ? 1 : 0;
        } else if (tile == LADDER_TILE_NUMBER) {
            value = property == "ladder" ? 1 : 0;
        } else if (property != "grab") {
            // QUIRK: for "grab" (and "ladder") on an ordinary tile the value of the previous layer is kept.
            if (property == "thru") {
                value = inList("thruTileList", tile) ? 1 : 0;
            } else if (property == "slope" || property == "solidslope") {
                value = firstMatch(floorSlopes, std::size(floorSlopes), tile);
                if (property == "solidslope" && value != 0 && !inList("slopeTileList_solid", tile)) {
                    value = 0;
                }
            } else if (property == "ceilingslope") {
                value = firstMatch(ceilingSlopes, std::size(ceilingSlopes), tile);
            } else if (property == "ceilingslopereverse") {
                value = firstMatch(reverseSlopes, std::size(reverseSlopes), tile);
                if (value != 0 && inList("slopeTileList_solid", tile)) {
                    value = 0;
                }
            } else if (property == "collision" || property == "naturalcollision") {
                value = inList("solidTileList", tile) ? 1 : 0;
            }
            if (value == -1) {
                value = 0;
            }
        }
        if (value > 0) {
            break;
        }
    }
    return value;
}

// AS: DataManager.getOutfitPrice
double DataManager::getOutfitPrice(double customer, double outfit) const
{
    if (customer >= 0 && customer < static_cast<double>(outfitPrices.size())) {
        const auto &prices = outfitPrices[static_cast<size_t>(customer)];
        if (outfit >= 0 && outfit < static_cast<double>(prices.size())) {
            return prices[static_cast<size_t>(outfit)];
        }
        return 999999;
    }
    return 999999;
}

// AS: DataManager.getEnemyName — by BADDIES index, else by enemy id; "???" when none.
std::string DataManager::getEnemyName(double index, double id) const
{
    std::string result = "???";
    if (index != -1 && index < static_cast<double>(enemyKey.size())) {
        result = enemyKey[static_cast<size_t>(index)].title;
    } else {
        for (const EnemyKeyEntry &k : enemyKey) {
            if (k.id == id) {
                result = k.title;
                break;
            }
        }
    }
    return result;
}

// AS: DataManager.getEnemyClipName
std::string DataManager::getEnemyClipName(double index, double id) const
{
    std::string result = "???";
    if (index != -1 && index < static_cast<double>(enemyKey.size())) {
        result = enemyKey[static_cast<size_t>(index)].clip;
    } else {
        for (const EnemyKeyEntry &k : enemyKey) {
            if (k.id == id) {
                result = k.clip;
                break;
            }
        }
    }
    return result;
}

// AS: DataManager.getEnemyIDFromIndex
double DataManager::getEnemyIDFromIndex(double index) const
{
    if (index > -1 && index < static_cast<double>(enemyKey.size())) {
        return enemyKey[static_cast<size_t>(index)].id;
    }
    return -1;
}

} // namespace papaGame::data
