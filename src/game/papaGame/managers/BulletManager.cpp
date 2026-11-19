#include "game/papaGame/managers/BulletManager.h"

#include <SDL3/SDL.h>

#include <algorithm>

#include "engine/json.h"
#include "game/obf_9G/obf_4R.h"
#include "game/papaGame/models/bullets/Basketball.h"

namespace papaGame::managers {

namespace {

// Port: getDefinitionByName("papaGame.models.bullets." + type).
std::unique_ptr<models::Bullet> createBullet(obf_9G::obf_4R *gameObj, const std::string &type)
{
    using namespace models::bullets;
    if (type == "Basketball") {
        return std::make_unique<Basketball>(gameObj);
    }
    return nullptr; // SKELETON: the other bullet classes are not ported yet
}

} // namespace

// AS: BulletManager.BulletManager
BulletManager::BulletManager(obf_9G::obf_4R *gameObj_) : gameObj(gameObj_) {}

BulletManager::~BulletManager() = default;

bool BulletManager::loadTables(const std::string &path)
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
        models::parseCycleTable(fields, tables_[name]);
    }
    return true;
}

// AS: BulletManager.addBullet
models::Bullet *BulletManager::addBullet(double x, double y, double dirx, double diry, const std::string &type,
                                         const std::string & /*package*/, bool /*unused*/, double statAttack,
                                         bool fromEnemy, const std::string &fromWeaponName)
{
    ++counter;
    const double id = counter;
    std::unique_ptr<models::Bullet> bullet = createBullet(gameObj, type);
    if (!bullet) {
        SDL_Log("ERROR: bullet %s does not exist", type.c_str());
        return nullptr;
    }
    bullet->cycleTable = &tables_[type];
    bullet->init(x, y, dirx, diry, type, id, statAttack, fromEnemy, fromWeaponName);
    bullet->savePrevious(); // port: nothing to interpolate from yet
    models::Bullet *b = bullet.get();
    byID_[id] = std::move(bullet);
    bullets.push_back(b);
    auto it = std::find(bulletTypes.begin(), bulletTypes.end(), type);
    double sheetIndex = static_cast<double>(it - bulletTypes.begin());
    if (it == bulletTypes.end()) {
        // loadBitmap / loadBitmapFromMovieClip: the port's sheets are converted at import.
        SDL_Log("Bullet sheet didn't exist!  Made a new one (%s)", type.c_str());
        bulletTypes.push_back(type);
        bulletSheets.push_back(gameObj->bitmapManager->sheets->get(b->sheetname));
        sheetIndex = static_cast<double>(bulletSheets.size()) - 1;
    }
    b->setSheetIndex(sheetIndex);
    return b;
}

// AS: BulletManager.processBullets — last to first.
void BulletManager::processBullets()
{
    graveyard_.clear();
    for (int i = static_cast<int>(bullets.size()) - 1; i >= 0; --i) {
        if (i < static_cast<int>(bullets.size()) && bullets[static_cast<size_t>(i)]) {
            bullets[static_cast<size_t>(i)]->updateObject();
        }
    }
}

// AS: BulletManager.clearBullets
void BulletManager::clearBullets()
{
    for (models::Bullet *b : bullets) {
        if (b) {
            b->destroy();
        }
    }
    for (auto &[id, b] : byID_) {
        graveyard_.push_back(std::move(b));
    }
    byID_.clear();
    counter = 0;
    bullets.clear();
    bulletSheets.clear();
    bulletTypes.clear();
    preblitSheetNames.clear();
}

// AS: BulletManager.addToPreblit
void BulletManager::addToPreblit(const std::string &type)
{
    SDL_Log("Add %s to preblit bullets", type.c_str());
    if (std::find(bulletTypes.begin(), bulletTypes.end(), type) == bulletTypes.end()) {
        bulletTypes.push_back(type);
        preblitSheetNames.push_back("bullet_" + type);
    }
}

// AS: BulletManager.preblitSheets
void BulletManager::preblitSheets()
{
    SDL_Log("Preblit %zu bullet sheets", preblitSheetNames.size());
    for (const std::string &name : preblitSheetNames) {
        bulletSheets.push_back(gameObj->bitmapManager->sheets->get(name));
    }
    preblitSheetNames.clear();
}

// AS: BulletManager.removeBullet
void BulletManager::removeBullet(double bulletid, bool nosplice)
{
    auto it = byID_.find(bulletid);
    models::Bullet *b = it != byID_.end() ? it->second.get() : nullptr;
    auto where = std::find(bullets.begin(), bullets.end(), b);
    if (b && where != bullets.end()) {
        *where = nullptr;
        if (!nosplice) {
            bullets.erase(where);
        }
    } else {
        SDL_Log("ERROR: Error removing bullet! Could not find Bullet%s", as3::String(bulletid).c_str());
    }
    if (b) {
        b->destroy();
        graveyard_.push_back(std::move(it->second));
        byID_.erase(it);
    }
}

} // namespace papaGame::managers
