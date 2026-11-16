#include "game/papaGame/managers/ChallengeManager.h"

#include <SDL3/SDL.h>

#include "engine/json.h"
#include "game/as3.h"

namespace papaGame::managers {

namespace {

// `new Challenge(whichWorld, whichChallenge, type, params)` as the importer wrote it; the setup* methods copy
// the params they use.
data::Challenge readChallenge(const engine::JsonValue &v)
{
    data::Challenge c;
    const engine::JsonValue *args = v.get("args");
    if (!args || args->items.size() < 4) {
        return c;
    }
    c.whichWorld = args->items[0].number;
    c.whichChallenge = args->items[1].number;
    c.challengeType = args->items[2].string;
    const engine::JsonValue &params = args->items[3];
    auto str = [&](const char *key, std::string &out) {
        if (const engine::JsonValue *p = params.get(key)) {
            out = p->string;
        }
    };
    auto num = [&](const char *key, double &out) {
        if (const engine::JsonValue *p = params.get(key)) {
            out = p->number;
        }
    };
    str("title", c.title);
    str("description", c.description);
    str("skillNeeded", c.skillNeeded);
    str("tagName", c.tagName);
    num("targetAmount", c.targetAmount);
    num("whichCustomer", c.whichCustomer);
    num("rewardMoney", c.rewardMoney);
    if (const engine::JsonValue *p = params.get("showTally")) {
        c.showTally = p->number != 0 || p->boolean;
    }
    return c;
}

} // namespace

bool ChallengeManager::loadTables(const std::string &path)
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
    challenges.clear();
    badges.clear();
    if (const engine::JsonValue *list = json.get("challenges")) {
        for (const engine::JsonValue &v : list->items) {
            challenges.push_back(readChallenge(v));
        }
    }
    if (const engine::JsonValue *list = json.get("badges")) {
        for (const engine::JsonValue &v : list->items) {
            badges.push_back(readChallenge(v));
        }
    }
    return !challenges.empty();
}

// AS: ChallengeManager.getChallengeTargetAmount
std::string ChallengeManager::getChallengeTargetAmount(double whichWorld, double whichChallenge) const
{
    if (whichWorld > -1) {
        for (const data::Challenge &c : challenges) {
            if (c.whichWorld == whichWorld && c.whichChallenge == whichChallenge) {
                return as3::String(c.targetAmount);
            }
        }
    }
    return {};
}

// AS: ChallengeManager.getMedalProgressArray
std::vector<double> ChallengeManager::getMedalProgressArray() const
{
    std::vector<double> out;
    for (const data::Challenge &badge : badges) {
        out.push_back(badge.tally);
    }
    return out;
}

// AS: ChallengeManager.populateMedalProgress
void ChallengeManager::populateMedalProgress(const std::vector<double> &progress)
{
    for (size_t i = 0; i < progress.size(); ++i) {
        if (i < badges.size()) {
            badges[i].tally = progress[i]; // Challenge.setTally
        }
    }
}

// AS: ChallengeManager.resetAllTallies (Challenge.clearTally(true) on every badge and challenge)
void ChallengeManager::resetAllTallies()
{
    for (data::Challenge &badge : badges) {
        badge.tally = 0;
    }
    for (data::Challenge &challenge : challenges) {
        challenge.tally = 0;
    }
}

} // namespace papaGame::managers
