#include "game/papaGame/managers/ObjectManager.h"

#include <SDL3/SDL.h>

#include "engine/json.h"
#include "game/obf_9G/obf_4R.h"
#include "game/papaGame/models/objects/GameObject1.h"
#include "game/papaGame/models/objects/GameObject8.h"
#include "game/papaGame/models/objects/GameObject9.h"
#include "game/papaGame/models/objects/GameObject10.h"
#include "game/papaGame/models/objects/GameObject11.h"
#include "game/papaGame/models/objects/GameObject26.h"
#include "game/papaGame/models/objects/GameObject27.h"
#include "game/papaGame/models/objects/GameObject12.h"
#include "game/papaGame/models/objects/GameObject13.h"
#include "game/papaGame/models/objects/GameObject14.h"
#include "game/papaGame/models/objects/GameObject17.h"
#include "game/papaGame/models/objects/GameObject18.h"
#include "game/papaGame/models/objects/GameObject19.h"
#include "game/papaGame/models/objects/GameObject20.h"
#include "game/papaGame/models/objects/GameObject21.h"
#include "game/papaGame/models/objects/GameObject22.h"
#include "game/papaGame/models/objects/GameObject23.h"
#include "game/papaGame/models/objects/GameObject24.h"
#include "game/papaGame/models/objects/GameObject25.h"
#include "game/papaGame/models/objects/GameObject28.h"
#include "game/papaGame/models/objects/GameObject2.h"
#include "game/papaGame/models/objects/GameObject3.h"
#include "game/papaGame/models/objects/GameObject29.h"
#include "game/papaGame/models/objects/GameObject30.h"
#include "game/papaGame/models/objects/GameObject33.h"
#include "game/papaGame/models/objects/GameObject4.h"
#include "game/papaGame/models/objects/GameObject5.h"
#include "game/papaGame/models/objects/GameObject6.h"
#include "game/papaGame/models/objects/GameObject7.h"

namespace papaGame::managers {

namespace {

// Port: getDefinitionByName("papaGame.models.objects.GameObject" + type).
std::unique_ptr<models::GameObject> createObject(obf_9G::obf_4R *gameObj, double type)
{
    using namespace models::objects;
    switch (static_cast<int>(type)) {
    case 1: return std::make_unique<GameObject1>(gameObj);
    case 2: return std::make_unique<GameObject2>(gameObj);
    case 3: return std::make_unique<GameObject3>(gameObj);
    case 4: return std::make_unique<GameObject4>(gameObj);
    case 5: return std::make_unique<GameObject5>(gameObj);
    case 6: return std::make_unique<GameObject6>(gameObj);
    case 7: return std::make_unique<GameObject7>(gameObj);
    case 8: return std::make_unique<GameObject8>(gameObj);
    case 9: return std::make_unique<GameObject9>(gameObj);
    case 10: return std::make_unique<GameObject10>(gameObj);
    case 11: return std::make_unique<GameObject11>(gameObj);
    case 12: return std::make_unique<GameObject12>(gameObj);
    case 13: return std::make_unique<GameObject13>(gameObj);
    case 14: return std::make_unique<GameObject14>(gameObj);
    case 17: return std::make_unique<GameObject17>(gameObj);
    case 18: return std::make_unique<GameObject18>(gameObj);
    case 19: return std::make_unique<GameObject19>(gameObj);
    case 20: return std::make_unique<GameObject20>(gameObj);
    case 21: return std::make_unique<GameObject21>(gameObj);
    case 22: return std::make_unique<GameObject22>(gameObj);
    case 23: return std::make_unique<GameObject23>(gameObj);
    case 24: return std::make_unique<GameObject24>(gameObj);
    case 25: return std::make_unique<GameObject25>(gameObj);
    case 26: return std::make_unique<GameObject26>(gameObj);
    case 27: return std::make_unique<GameObject27>(gameObj);
    case 28: return std::make_unique<GameObject28>(gameObj);
    case 29: return std::make_unique<GameObject29>(gameObj);
    case 30: return std::make_unique<GameObject30>(gameObj);
    case 33: return std::make_unique<GameObject33>(gameObj);
    default: return nullptr; // SKELETON: the other object types are not ported yet
    }
}

} // namespace

ObjectManager::ObjectManager(obf_9G::obf_4R *gameObj_) : gameObj(gameObj_) {}

ObjectManager::~ObjectManager() = default;

bool ObjectManager::loadTables(const std::string &path)
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
    if (const engine::JsonValue *base = ok ? json.get("GameObject") : nullptr) {
        models::GameObject::loadTables(*base);
        return true;
    }
    return false;
}

// AS: ObjectManager.updateTrainingSigns — after the keys were changed.
void ObjectManager::updateTrainingSigns()
{
    for (models::GameObject *o : objects) {
        if (o) {
            o->updateTrainingSign();
        }
    }
}

// AS: ObjectManager.getObject
models::GameObject *ObjectManager::getObject(double screenObjID, double whichScreen) const
{
    for (models::GameObject *o : objects) {
        if (o && o->screenObjID == screenObjID && o->whichScreen == whichScreen) {
            return o;
        }
    }
    return nullptr;
}

// AS: ObjectManager.checkSteppingOnObjects — byObject: an object looking for objects that carry objects.
void ObjectManager::checkSteppingOnObjects(models::Character &c, bool byObject)
{
    bool alreadyOnObject = false;
    if (c.onMovingTile) {
        alreadyOnObject = true;
        c.checkMovingTiles(1);
    }
    for (size_t i = 0; i < objects.size(); ++i) {
        if (models::GameObject *o = objects[i]) {
            if (o->isSteppable && (!byObject || o->isSteppableByObjects)) {
                o->checkStartStepping(c, alreadyOnObject);
            }
        }
    }
}

// Port: a character still standing on a freed object lets go of it (the original keeps the stale reference, and
// its next checkMovingTiles finds it is no longer standing there).
void ObjectManager::releaseStandingChars(models::GameObject &o)
{
    if (gameObj->playerObj && gameObj->playerObj->whichMovingTile == &o) {
        gameObj->playerObj->whichMovingTile = nullptr;
        gameObj->playerObj->onMovingTile = false;
    }
    for (models::Enemy *e : gameObj->obf_E->enemies) {
        if (e && e->whichMovingTile == &o) {
            e->whichMovingTile = nullptr;
            e->onMovingTile = false;
        }
    }
    for (models::GameObject *other : objects) {
        if (other && other->whichMovingTile == &o) {
            other->whichMovingTile = nullptr;
            other->onMovingTile = false;
        }
    }
}

// AS: ObjectManager.addObject — returns the new object's id.
double ObjectManager::addObject(double xtile, double ytile, double type, double whichScreen, double screenObjID,
                                const std::vector<as3::Value> &extraParams)
{
    ++counter;
    const double objectid = counter;
    if (screenObjID > maxScreenID) {
        maxScreenID = screenObjID;
    }
    std::unique_ptr<models::GameObject> object = createObject(gameObj, type);
    if (!object) {
        SDL_Log("OBJECT DOESN'T EXIST: GameObject%s", as3::String(type).c_str());
        return objectid;
    }
    object->init(xtile, ytile, objectid, whichScreen, screenObjID, extraParams);
    models::GameObject *o = object.get();
    byID_[objectid] = std::move(object);
    objects.push_back(o);
    double sheetIndex = -1;
    for (size_t i = 0; i < objectTypes.size(); ++i) {
        if (objectTypes[i] == type) {
            sheetIndex = static_cast<double>(i);
            break;
        }
    }
    if (sheetIndex == -1) {
        objectTypes.push_back(type);
        objectSheets.push_back(gameObj->bitmapManager->sheets->get(o->sheetname));
        sheetIndex = static_cast<double>(objectSheets.size()) - 1;
    }
    o->setSheetIndex(sheetIndex);
    return objectid;
}

// AS: ObjectManager.processObjects — last to first.
void ObjectManager::processObjects()
{
    graveyard_.clear();
    for (int i = static_cast<int>(objects.size()) - 1; i >= 0; --i) {
        if (i < static_cast<int>(objects.size()) && objects[static_cast<size_t>(i)]) {
            objects[static_cast<size_t>(i)]->updateObject();
        }
    }
}

// AS: ObjectManager.clearObjects (SKELETON: a carried object travels to the next room)
void ObjectManager::clearObjects(double whichScreen, bool all, bool /*resetCounter*/, bool /*keepCarried*/)
{
    for (int i = static_cast<int>(objects.size()) - 1; i >= 0; --i) {
        models::GameObject *o = objects[static_cast<size_t>(i)];
        if (o && (o->whichScreen == whichScreen || all)) {
            o->destroy();
            releaseStandingChars(*o);
            auto it = byID_.find(o->id);
            if (it != byID_.end()) {
                graveyard_.push_back(std::move(it->second));
                byID_.erase(it);
            }
            objects.erase(objects.begin() + i);
        }
    }
    maxScreenID = 0;
    if (all) {
        counter = 0;
        objects.clear();
    }
    objectSheets.clear();
    objectTypes.clear();
}

// AS: ObjectManager.saveObjects
void ObjectManager::saveObjects()
{
    clearObjects(1, true);
}

// AS: ObjectManager.buildObjects — rows of objectArray: [type, x, y, triggers, params...]; type 0 is gone.
void ObjectManager::buildObjects(double whichScreen)
{
    const Uint64 start = SDL_GetTicks();
    const display::GameDisplay &d = *gameObj->obf_71;
    const double offsetX = whichScreen * d.screenTileWidth;
    auto &screen = *gameObj->obf_AQ->currentScreenData;
    maxScreenID = -1;
    const auto &rows = screen.objectArray;
    for (size_t i = 0; i < rows.size(); ++i) {
        const auto &row = rows[i];
        const double type = row.empty() ? 0 : row[0].asNumber();
        if (type != 0) {
            const double x = offsetX + (row.size() > 1 ? row[1].asNumber() : 0);
            const double y = row.size() > 2 ? row[2].asNumber() : 0;
            std::vector<as3::Value> params;
            if (row.size() > 4) {
                params.assign(row.begin() + 4, row.end());
            }
            addObject(x, y, type, whichScreen, static_cast<double>(i) + 1, params);
        } else {
            ++counter;
        }
    }
    screen.objectTotal = static_cast<double>(rows.size());
    // SKELETON: recreateCarriedObject.
    lastBuildDuration = static_cast<double>(SDL_GetTicks() - start);
}

// AS: ObjectManager.removeObject
void ObjectManager::removeObject(double objectID, bool nosplice)
{
    auto it = byID_.find(objectID);
    if (it == byID_.end()) {
        return;
    }
    models::GameObject *o = it->second.get();
    for (size_t i = 0; i < objects.size(); ++i) {
        if (objects[i] == o) {
            objects[i] = nullptr;
            if (!nosplice) {
                objects.erase(objects.begin() + static_cast<std::ptrdiff_t>(i));
            }
            o->destroy();
            releaseStandingChars(*o);
            graveyard_.push_back(std::move(it->second));
            byID_.erase(it);
            return;
        }
    }
}

// AS: ObjectManager.activateObject
void ObjectManager::activateObject(double whichScreen, double screenObjID, bool pan, double amount, bool fromGlobal)
{
    if (models::GameObject *o = getObject(screenObjID, whichScreen)) {
        o->sendTriggered(amount, pan, fromGlobal);
    }
}

// AS: ObjectManager.setEvent
void ObjectManager::setEvent(double whichScreen, double screenObjID, const std::vector<std::vector<double>> &triggers,
                             double pan, double panTarget)
{
    if (models::GameObject *o = getObject(screenObjID, whichScreen)) {
        o->setEventData(triggers, pan, panTarget);
    }
}

// AS: ObjectManager.setEventTriggers
void ObjectManager::setEventTriggers(double whichScreen, double screenObjID, double necessary,
                                     const std::vector<std::vector<double>> &from)
{
    if (models::GameObject *o = getObject(screenObjID, whichScreen)) {
        o->setEventTriggers(necessary, from);
    }
}

} // namespace papaGame::managers
