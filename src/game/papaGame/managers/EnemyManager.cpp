#include "game/papaGame/managers/EnemyManager.h"

#include <SDL3/SDL.h>

#include "engine/json.h"
#include "game/obf_9G/obf_4R.h"
#include "game/papaGame/models/enemies/Enemy1.h"
#include "game/papaGame/models/enemies/Enemy2.h"
#include "game/papaGame/models/enemies/Enemy3.h"
#include "game/papaGame/models/enemies/Enemy34.h"
#include "game/papaGame/models/enemies/Enemy4.h"
#include "game/papaGame/models/enemies/Enemy6.h"
#include "game/papaGame/models/enemies/Enemy5.h"
#include "game/papaGame/models/enemies/Enemy7.h"
#include "game/papaGame/models/enemies/Enemy12.h"
#include "game/papaGame/models/enemies/Enemy14.h"
#include "game/papaGame/models/enemies/Enemy15.h"
#include "game/papaGame/models/enemies/Enemy16.h"
#include "game/papaGame/models/enemies/Enemy17.h"
#include "game/papaGame/models/enemies/Enemy13.h"
#include "game/papaGame/models/enemies/Enemy19.h"
#include "game/papaGame/models/enemies/Enemy25.h"
#include "game/papaGame/models/enemies/Enemy26.h"
#include "game/papaGame/models/enemies/Enemy32.h"
#include "game/papaGame/models/enemies/Enemy35.h"
#include "game/papaGame/models/enemies/Enemy8.h"
#include "game/papaGame/models/enemies/Enemy9.h"
#include "game/papaGame/models/enemies/Enemy10.h"
#include "game/papaGame/models/enemies/Enemy11.h"

namespace papaGame::managers {

namespace {

// Port: getDefinitionByName("papaGame.models.enemies.Enemy" + type).
std::unique_ptr<models::Enemy> createEnemy(obf_9G::obf_4R *gameObj, double type)
{
    using namespace models::enemies;
    switch (static_cast<int>(type)) {
    case 1: return std::make_unique<Enemy1>(gameObj);
    case 2: return std::make_unique<Enemy2>(gameObj);
    case 3: return std::make_unique<Enemy3>(gameObj);
    case 4: return std::make_unique<Enemy4>(gameObj);
    case 6: return std::make_unique<Enemy6>(gameObj);
    case 5: return std::make_unique<Enemy5>(gameObj);
    case 7: return std::make_unique<Enemy7>(gameObj);
    case 8: return std::make_unique<Enemy8>(gameObj);
    case 9: return std::make_unique<Enemy9>(gameObj);
    case 10: return std::make_unique<Enemy10>(gameObj);
    case 11: return std::make_unique<Enemy11>(gameObj);
    case 12: return std::make_unique<Enemy12>(gameObj);
    case 14: return std::make_unique<Enemy14>(gameObj);
    case 15: return std::make_unique<Enemy15>(gameObj);
    case 16: return std::make_unique<Enemy16>(gameObj);
    case 17: return std::make_unique<Enemy17>(gameObj);
    case 13: return std::make_unique<Enemy13>(gameObj);
    case 19: return std::make_unique<Enemy19>(gameObj);
    case 25: return std::make_unique<Enemy25>(gameObj);
    case 26: return std::make_unique<Enemy26>(gameObj);
    case 32: return std::make_unique<Enemy32>(gameObj);
    case 35: return std::make_unique<Enemy35>(gameObj);
    case 34: return std::make_unique<Enemy34>(gameObj);
    default: return nullptr; // SKELETON: the other enemy types are not ported yet
    }
}

} // namespace

EnemyManager::EnemyManager(obf_9G::obf_4R *gameObj_) : gameObj(gameObj_) {}

EnemyManager::~EnemyManager() = default;

bool EnemyManager::loadTables(const std::string &path)
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
    tables_.clear();
    for (const auto &[name, fields] : json.fields) {
        if (name == "Enemy") {
            models::Enemy::loadTables(fields);
        } else {
            models::parseCycleTable(fields, tables_[name]);
        }
    }
    // Port: the importer keeps each class's own arrays; a subclass also gets its parent's.
    static const std::pair<const char *, const char *> parents[] = {
        {"Enemy4", "Enemy3"}, {"Enemy10", "Enemy9"}, {"Enemy11", "Enemy9"}, {"Enemy39", "Enemy38"}};
    for (const auto &[child, parent] : parents) {
        for (const auto &[field, cycle] : tables_[parent]) {
            tables_[child].emplace(field, cycle);
        }
    }
    return true;
}

// AS: EnemyManager.addEnemy — returns the new enemy's id.
double EnemyManager::addEnemy(double xtile, double ytile, double type, double whichScreen, double whichTileset,
                              double screenObjID, const std::vector<as3::Value> &extraParams)
{
    ++counter;
    const double enemyid = counter;
    std::unique_ptr<models::Enemy> enemy = createEnemy(gameObj, type);
    if (!enemy) {
        SDL_Log("ENEMY DOESN'T EXIST: Enemy%s", as3::String(type).c_str());
        return enemyid;
    }
    enemy->cycleTable = &tables_["Enemy" + as3::String(type)];
    enemy->init(xtile, ytile, enemyid, whichScreen, whichTileset, screenObjID, extraParams);
    models::Enemy *e = enemy.get();
    byID_[enemyid] = std::move(enemy);
    enemies.push_back(e);
    double sheetIndex = -1;
    for (size_t i = 0; i < objectTypes.size(); ++i) {
        if (objectTypes[i] == type) {
            sheetIndex = static_cast<double>(i);
            break;
        }
    }
    if (sheetIndex == -1) {
        // loadBitmap / loadBitmapFromMovieClip: the port's sheets are baked by the importer.
        objectTypes.push_back(type);
        objectSheets.push_back(gameObj->bitmapManager->sheets->get(e->sheetname));
        sheetIndex = static_cast<double>(objectSheets.size()) - 1;
    }
    e->setSheetIndex(sheetIndex);
    return enemyid;
}

// AS: EnemyManager.processEnemies — last to first, so enemies can remove themselves.
void EnemyManager::processEnemies()
{
    graveyard_.clear();
    for (int i = static_cast<int>(enemies.size()) - 1; i >= 0; --i) {
        if (i < static_cast<int>(enemies.size()) && enemies[static_cast<size_t>(i)]) {
            enemies[static_cast<size_t>(i)]->updateObject();
        }
    }
}

// AS: EnemyManager.clearEnemies
void EnemyManager::clearEnemies(double whichScreen, bool all, bool resetCounter)
{
    for (int i = static_cast<int>(enemies.size()) - 1; i >= 0; --i) {
        models::Enemy *e = enemies[static_cast<size_t>(i)];
        if (e && (e->whichScreen == whichScreen || all)) {
            e->destroy();
            auto it = byID_.find(e->id);
            if (it != byID_.end()) {
                graveyard_.push_back(std::move(it->second));
                byID_.erase(it);
            }
            enemies.erase(enemies.begin() + i);
        }
    }
    if (resetCounter) {
        counter = static_cast<double>(enemies.size());
    } else if (all) {
        counter = 0;
        enemies.clear();
    }
    objectSheets.clear();
    objectTypes.clear();
    preblitSheetNames.clear();
    preblitObjectTypes.clear();
}

// AS: EnemyManager.saveEnemies
void EnemyManager::saveEnemies()
{
    clearEnemies(1, true);
}

// AS: EnemyManager.buildEnemies — rows of enemyArray: [type, x, y, params...]; type 0 is a killed enemy.
void EnemyManager::buildEnemies(double whichScreen)
{
    const Uint64 start = SDL_GetTicks();
    const display::GameDisplay &d = *gameObj->obf_71;
    const double offsetX = whichScreen * d.screenTileWidth;
    const auto &rows = gameObj->obf_AQ->currentScreenData->enemyArray;
    for (size_t i = 0; i < rows.size(); ++i) {
        const auto &row = rows[i];
        const double type = row.empty() ? 0 : row[0].asNumber();
        if (type != 0) {
            const double x = offsetX + (row.size() > 1 ? row[1].asNumber() : 0);
            const double y = row.size() > 2 ? row[2].asNumber() : 0;
            std::vector<as3::Value> params;
            if (row.size() > 3) {
                params.assign(row.begin() + 3, row.end());
            }
            addEnemy(x, y, type, whichScreen, 1, static_cast<double>(i) + 1, params);
        } else {
            ++counter;
        }
    }
    lastBuildDuration = static_cast<double>(SDL_GetTicks() - start);
}

// AS: EnemyManager.removeEnemy
void EnemyManager::removeEnemy(double enemyID, bool nosplice)
{
    auto it = byID_.find(enemyID);
    models::Enemy *e = it == byID_.end() ? nullptr : it->second.get();
    // QUIRK: with no match, `which` stays NaN and enemies[NaN] = 0 sets a property, not an element.
    for (size_t i = 0; e && i < enemies.size(); ++i) {
        if (enemies[i] == e) {
            enemies[i] = nullptr;
            if (!nosplice) {
                enemies.erase(enemies.begin() + static_cast<std::ptrdiff_t>(i));
            }
            break;
        }
    }
    if (e) {
        e->destroy();
        graveyard_.push_back(std::move(it->second));
        byID_.erase(it);
    }
}

// AS: EnemyManager.getEnemy
models::Enemy *EnemyManager::getEnemy(double screenObjID, double whichScreen) const
{
    for (models::Enemy *e : enemies) {
        if (e && e->screenObjID == screenObjID && e->whichScreen == whichScreen) {
            return e;
        }
    }
    return nullptr;
}

// AS: EnemyManager.sendMessageToEnemyGroup — to every other enemy of the group, last to first.
void EnemyManager::sendMessageToEnemyGroup(double fromID, double groupID, const std::string &message)
{
    for (int i = static_cast<int>(enemies.size()) - 1; i >= 0; --i) {
        models::Enemy *e = i < static_cast<int>(enemies.size()) ? enemies[static_cast<size_t>(i)] : nullptr;
        if (e && e->groupID == groupID && e->id != fromID) {
            e->receiveGroupMessage(message, fromID);
        }
    }
}

// AS: EnemyManager.getEnemyIDsFromGroup — the ids of the group (fromID is not used), last to first.
std::vector<double> EnemyManager::getEnemyIDsFromGroup(double /*fromID*/, double groupID) const
{
    std::vector<double> ids;
    for (int i = static_cast<int>(enemies.size()) - 1; i >= 0; --i) {
        const models::Enemy *e = enemies[static_cast<size_t>(i)];
        if (e && e->groupID == groupID) {
            ids.push_back(e->id);
        }
    }
    return ids;
}

// AS: EnemyManager.checkGroupOnScreen — is any enemy of these ids still there and on screen?
bool EnemyManager::checkGroupOnScreen(const std::vector<double> &ids) const
{
    for (double enemyID : ids) {
        const auto it = byID_.find(enemyID);
        if (it != byID_.end() && it->second && gameObj->obf_71->checkOnScreen(*it->second)) {
            return true;
        }
    }
    return false;
}

// AS: EnemyManager.checkEnemyTypeOnScreen — is any enemy of this type on screen?
bool EnemyManager::checkEnemyTypeOnScreen(double type) const
{
    const display::GameDisplay &d = *gameObj->obf_71;
    for (models::Enemy *e : enemies) {
        if (e && e->type == type && d.checkOnScreen(*e)) {
            return true;
        }
    }
    return false;
}

// AS: EnemyManager.addToPreblit
void EnemyManager::addToPreblit(double type, const std::string &sheetName)
{
    for (double t : preblitObjectTypes) {
        if (t == type) {
            return;
        }
    }
    preblitObjectTypes.push_back(type);
    preblitSheetNames.push_back(sheetName);
}

// AS: EnemyManager.preblitSheets
void EnemyManager::preblitSheets()
{
    for (size_t i = 0; i < preblitSheetNames.size(); ++i) {
        bool known = false;
        for (double t : objectTypes) {
            known = known || t == preblitObjectTypes[i];
        }
        if (!known) {
            objectSheets.push_back(gameObj->bitmapManager->sheets->get(preblitSheetNames[i]));
            objectTypes.push_back(preblitObjectTypes[i]);
        }
    }
    preblitSheetNames.clear();
    preblitObjectTypes.clear();
}

// AS: EnemyManager.activateObject
void EnemyManager::activateObject(double /*whichScreen*/, double enemyID, bool pan, double amount, bool fromGlobal)
{
    auto it = byID_.find(enemyID);
    if (it != byID_.end()) {
        it->second->sendTriggered(amount, pan, fromGlobal);
    }
}

// AS: EnemyManager.setEvent
void EnemyManager::setEvent(double /*whichScreen*/, double enemyID, const std::vector<std::vector<double>> &triggers,
                            double pan, double panTarget)
{
    auto it = byID_.find(enemyID);
    if (it != byID_.end()) {
        it->second->setEventData(triggers, pan, panTarget);
    }
}

// AS: EnemyManager.setEventTriggers
void EnemyManager::setEventTriggers(double /*whichScreen*/, double enemyID, double necessary,
                                    const std::vector<std::vector<double>> &from)
{
    auto it = byID_.find(enemyID);
    if (it != byID_.end()) {
        it->second->setEventTriggers(necessary, from);
    }
}

} // namespace papaGame::managers
