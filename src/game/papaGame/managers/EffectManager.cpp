#include "game/papaGame/managers/EffectManager.h"

#include <SDL3/SDL.h>

#include "engine/json.h"
#include "game/obf_9G/obf_4R.h"
#include "game/papaGame/models/effects/CageBackBarsEffect.h"
#include "game/papaGame/models/effects/CageFrontBarsEffect.h"
#include "game/papaGame/models/effects/CageSpinnerEffect.h"
#include "game/papaGame/models/effects/CookieBitEffect.h"
#include "game/papaGame/models/effects/CrackerBreakEffect.h"
#include "game/papaGame/models/effects/DizzyEffect.h"
#include "game/papaGame/models/effects/CageCustomerEffect.h"
#include "game/papaGame/models/effects/DoorFrontEffect.h"
#include "game/papaGame/models/effects/BossEscapeEffect.h"
#include "game/papaGame/models/effects/DustEffect.h"
#include "game/papaGame/models/effects/LaughEffect.h"
#include "game/papaGame/models/effects/EndPortalEffect.h"
#include "game/papaGame/models/effects/ExplosionEffect.h"
#include "game/papaGame/models/effects/ItemBalloonEffect.h"
#include "game/papaGame/models/effects/ItemCoinEffect.h"
#include "game/papaGame/models/effects/ItemDaisyEffect.h"
#include "game/papaGame/models/effects/ItemFizzoCanEffect.h"
#include "game/papaGame/models/effects/ItemGoldHelmetEffect.h"
#include "game/papaGame/models/effects/ItemGummyWormEffect.h"
#include "game/papaGame/models/effects/ItemHeartEffect.h"
#include "game/papaGame/models/effects/ItemPapaCoinEffect.h"
#include "game/papaGame/models/effects/ItemRadishCoinEffect.h"
#include "game/papaGame/models/effects/ItemSargeCoinEffect.h"
#include "game/papaGame/models/effects/PlayerCheckpointEffect.h"
#include "game/papaGame/models/effects/RomanSparkEffect.h"
#include "game/papaGame/models/effects/RomanTrailEffect.h"
#include "game/papaGame/models/effects/BBQBitEffect.h"
#include "game/papaGame/models/effects/BlueberryBitEffect.h"
#include "game/papaGame/models/effects/BlueberrySplatEffect.h"
#include "game/papaGame/models/effects/CheeseBitEffect.h"
#include "game/papaGame/models/effects/CheeseSplatEffect.h"
#include "game/papaGame/models/effects/RadishDirtEffect.h"
#include "game/papaGame/models/effects/SauceBitEffect.h"
#include "game/papaGame/models/effects/SauceSplatEffect.h"
#include "game/papaGame/models/effects/RainbowBitEffect.h"
#include "game/papaGame/models/effects/BeakerGlassEffect.h"
#include "game/papaGame/models/effects/StuckCheeseEffect.h"
#include "game/papaGame/models/effects/FeatherEffect.h"
#include "game/papaGame/models/effects/ChiliBitEffect.h"
#include "game/papaGame/models/effects/ChiliSplatEffect.h"
#include "game/papaGame/models/effects/SmokeRingEffect.h"
#include "game/papaGame/models/effects/StarEffect.h"
#include "game/papaGame/models/effects/StarburstEffect.h"
#include "game/papaGame/models/effects/ZillaPartsEffect.h"

namespace papaGame::managers {

namespace {

// Port: getDefinitionByName("papaGame.models.effects." + type).
std::unique_ptr<models::Effect> createEffect(obf_9G::obf_4R *gameObj, const std::string &type)
{
    using namespace models::effects;
    if (type == "DizzyEffect") {
        return std::make_unique<DizzyEffect>(gameObj);
    }
    if (type == "StarburstEffect") {
        return std::make_unique<StarburstEffect>(gameObj);
    }
    if (type == "CageCustomerEffect") {
        return std::make_unique<CageCustomerEffect>(gameObj);
    }
    if (type == "DoorFrontEffect") {
        return std::make_unique<DoorFrontEffect>(gameObj);
    }
    if (type == "LaughEffect") {
        return std::make_unique<LaughEffect>(gameObj);
    }
    if (type == "BossEscapeEffect") {
        return std::make_unique<BossEscapeEffect>(gameObj);
    }
    if (type == "DustEffect") {
        return std::make_unique<DustEffect>(gameObj);
    }
    if (type == "CrackerBreakEffect") {
        return std::make_unique<CrackerBreakEffect>(gameObj);
    }
    if (type == "ZillaPartsEffect") {
        return std::make_unique<ZillaPartsEffect>(gameObj);
    }
    if (type == "PlayerCheckpointEffect") {
        return std::make_unique<PlayerCheckpointEffect>(gameObj);
    }
    if (type == "EndPortalEffect") {
        return std::make_unique<EndPortalEffect>(gameObj);
    }
    if (type == "ItemCoinEffect") {
        return std::make_unique<ItemCoinEffect>(gameObj);
    }
    if (type == "ItemHeartEffect") {
        return std::make_unique<ItemHeartEffect>(gameObj);
    }
    if (type == "ItemPapaCoinEffect") {
        return std::make_unique<ItemPapaCoinEffect>(gameObj);
    }
    if (type == "ItemSargeCoinEffect") {
        return std::make_unique<ItemSargeCoinEffect>(gameObj);
    }
    if (type == "ItemRadishCoinEffect") {
        return std::make_unique<ItemRadishCoinEffect>(gameObj);
    }
    if (type == "ItemDaisyEffect") {
        return std::make_unique<ItemDaisyEffect>(gameObj);
    }
    if (type == "ItemBalloonEffect") {
        return std::make_unique<ItemBalloonEffect>(gameObj);
    }
    if (type == "ItemFizzoCanEffect") {
        return std::make_unique<ItemFizzoCanEffect>(gameObj);
    }
    if (type == "ItemGoldHelmetEffect") {
        return std::make_unique<ItemGoldHelmetEffect>(gameObj);
    }
    if (type == "ItemGummyWormEffect") {
        return std::make_unique<ItemGummyWormEffect>(gameObj);
    }
    if (type == "CageFrontBarsEffect") {
        return std::make_unique<CageFrontBarsEffect>(gameObj);
    }
    if (type == "CageBackBarsEffect") {
        return std::make_unique<CageBackBarsEffect>(gameObj);
    }
    if (type == "CageSpinnerEffect") {
        return std::make_unique<CageSpinnerEffect>(gameObj);
    }
    if (type == "CookieBitEffect") {
        return std::make_unique<CookieBitEffect>(gameObj);
    }
    if (type == "ExplosionEffect") {
        return std::make_unique<ExplosionEffect>(gameObj);
    }
    if (type == "RomanSparkEffect") {
        return std::make_unique<RomanSparkEffect>(gameObj);
    }
    if (type == "RomanTrailEffect") {
        return std::make_unique<RomanTrailEffect>(gameObj);
    }
    if (type == "StarEffect") {
        return std::make_unique<StarEffect>(gameObj);
    }
    if (type == "SmokeRingEffect") {
        return std::make_unique<SmokeRingEffect>(gameObj);
    }
    if (type == "CheeseBitEffect") {
        return std::make_unique<CheeseBitEffect>(gameObj);
    }
    if (type == "RadishDirtEffect") {
        return std::make_unique<RadishDirtEffect>(gameObj);
    }
    if (type == "RainbowBitEffect") {
        return std::make_unique<RainbowBitEffect>(gameObj);
    }
    if (type == "BeakerGlassEffect") {
        return std::make_unique<BeakerGlassEffect>(gameObj);
    }
    if (type == "SauceBitEffect") {
        return std::make_unique<SauceBitEffect>(gameObj);
    }
    if (type == "SauceSplatEffect") {
        return std::make_unique<SauceSplatEffect>(gameObj);
    }
    if (type == "CheeseSplatEffect") {
        return std::make_unique<CheeseSplatEffect>(gameObj);
    }
    if (type == "StuckCheeseEffect") {
        return std::make_unique<StuckCheeseEffect>(gameObj);
    }
    if (type == "ChiliBitEffect") {
        return std::make_unique<ChiliBitEffect>(gameObj);
    }
    if (type == "BBQBitEffect") {
        return std::make_unique<BBQBitEffect>(gameObj);
    }
    if (type == "ChiliSplatEffect") {
        return std::make_unique<ChiliSplatEffect>(gameObj);
    }
    if (type == "BlueberryBitEffect") {
        return std::make_unique<BlueberryBitEffect>(gameObj);
    }
    if (type == "BlueberrySplatEffect") {
        return std::make_unique<BlueberrySplatEffect>(gameObj);
    }
    if (type == "FeatherEffect") {
        return std::make_unique<FeatherEffect>(gameObj);
    }
    return nullptr; // SKELETON: the other effect classes are not ported yet
}

} // namespace

EffectManager::EffectManager(obf_9G::obf_4R *gameObj_) : gameObj(gameObj_) {}

EffectManager::~EffectManager() = default;

bool EffectManager::loadTables(const std::string &path)
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

// AS: EffectManager.addEffect
double EffectManager::addEffect(double x, double y, const std::string &type, const std::string &subtype,
                                bool followPlayer, double playerXoffset, double playerYoffset)
{
    ++counter;
    const double effectid = counter;
    std::unique_ptr<models::Effect> effect = createEffect(gameObj, type);
    if (!effect) {
        SDL_Log("ERROR: effect %s does not exist", type.c_str());
        return -1;
    }
    effect->cycleTable = &tables_[type];
    effect->init(x, y, effectid, subtype, followPlayer, playerXoffset, playerYoffset);
    models::Effect *e = effect.get();
    byID_[effectid] = std::move(effect);
    effects.push_back(e);
    double sheetIndex = -1;
    for (size_t i = 0; i < objectTypes.size(); ++i) {
        if (objectTypes[i] == type) {
            sheetIndex = static_cast<double>(i);
            break;
        }
    }
    if (sheetIndex == -1) {
        objectTypes.push_back(type);
        objectSheets.push_back(gameObj->bitmapManager->sheets->get(e->sheetname));
        sheetIndex = static_cast<double>(objectSheets.size()) - 1;
    }
    e->setSheetIndex(sheetIndex);
    return effectid;
}

// AS: EffectManager.processEffects — last to first.
void EffectManager::processEffects()
{
    graveyard_.clear();
    for (int i = static_cast<int>(effects.size()) - 1; i >= 0; --i) {
        if (i < static_cast<int>(effects.size()) && effects[static_cast<size_t>(i)]) {
            effects[static_cast<size_t>(i)]->updateObject();
        }
    }
}

// AS: EffectManager.clearEffects
void EffectManager::clearEffects()
{
    for (models::Effect *e : effects) {
        if (e) {
            e->destroy();
        }
    }
    for (auto &[id, e] : byID_) {
        graveyard_.push_back(std::move(e));
    }
    byID_.clear();
    counter = 0;
    effects.clear();
    objectSheets.clear();
    objectTypes.clear();
}

// AS: EffectManager.removeEffect
void EffectManager::removeEffect(double effectID, bool nosplice)
{
    for (size_t i = 0; i < effects.size(); ++i) {
        if (effects[i] && effects[i]->id == effectID) {
            effects[i]->destroy();
            auto it = byID_.find(effectID);
            if (it != byID_.end()) {
                graveyard_.push_back(std::move(it->second));
                byID_.erase(it);
            }
            effects[i] = nullptr;
            if (!nosplice) {
                effects.erase(effects.begin() + static_cast<std::ptrdiff_t>(i));
            }
            return;
        }
    }
}

// AS: EffectManager.getEffect
models::Effect *EffectManager::getEffect(double effectID) const
{
    for (models::Effect *e : effects) {
        if (e && e->id == effectID) {
            return e;
        }
    }
    return nullptr;
}

} // namespace papaGame::managers
