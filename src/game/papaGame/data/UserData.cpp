#include "game/papaGame/data/UserData.h"

#include <SDL3/SDL.h>

#include "engine/json.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::data {

namespace {

void refreshHUD(obf_9G::obf_4R *gameObj)
{
    if (gameObj->obf_6Z) {
        gameObj->obf_6Z->updateDisplay();
    }
}

} // namespace

// AS: UserData.adjustLives — false when no life is left.
bool UserData::adjustLives(double amount)
{
    totalLives += amount;
    refreshHUD(gameObj);
    if (amount < 0) {
        livesLost += -amount;
    }
    return totalLives >= 0;
}

// AS: UserData.earnPoints
void UserData::earnPoints(double amount)
{
    points += amount;
    refreshHUD(gameObj);
}

// AS: UserData.earnMoney
void UserData::earnMoney(double amount)
{
    money += amount;
    refreshHUD(gameObj);
}

// AS: UserData.collectSpecialItem (SKELETON: ChallengeManager.recordSpecialItem)
void UserData::collectSpecialItem(double amount)
{
    specialitems += amount;
    refreshHUD(gameObj);
}

// AS: UserData.killBurgerzilla (SKELETON: ChallengeManager.recordBurgerzilla)
void UserData::killBurgerzilla(double amount)
{
    burgerzillas += amount;
    refreshHUD(gameObj);
}

bool UserData::loadTables(const std::string &path)
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
    customersUnlocked.clear();
    if (const engine::JsonValue *list = json.get("customersUnlocked")) {
        for (const engine::JsonValue &v : list->items) {
            customersUnlocked.push_back(v.number);
        }
    }
    return true;
}

// AS: UserData.hasCustomerUnlocked
bool UserData::hasCustomerUnlocked(double index) const
{
    return index >= 0 && index < static_cast<double>(customersUnlocked.size()) &&
           customersUnlocked[static_cast<size_t>(index)] == 1;
}

// AS: UserData.unlockCustomer (SKELETON: ChallengeManager.recordTag("customerUnlocked") for a new one)
void UserData::unlockCustomer(double index)
{
    if (index >= 0 && index < static_cast<double>(customersUnlocked.size())) {
        customersUnlocked[static_cast<size_t>(index)] = 1;
    }
}

} // namespace papaGame::data
