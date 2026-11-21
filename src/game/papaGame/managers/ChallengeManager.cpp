#include "game/papaGame/managers/ChallengeManager.h"

#include <SDL3/SDL.h>

#include <functional>

#include "engine/json.h"
#include "game/as3.h"
#include "game/obf_AF/obf_81.h"
#include "game/obf_9G/obf_4R.h"

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
    str("whichWeapon", c.whichWeapon);
    str("weaponType", c.weaponType);
    num("enemyType", c.enemyType);
    str("enemyWeapon", c.enemyWeapon);
    str("enemyWeaponType", c.enemyWeaponType);
    if (params.get("enemyHeadName")) {
        str("enemyHeadName", c.enemyHeadName);
    } else {
        str("enemyHeadType", c.enemyHeadName);
    }
    str("enemyCategory", c.enemyCategory);
    num("juggleTimesHit", c.juggleTimesHit);
    num("juggleWeaponsAmount", c.juggleWeaponsAmount);
    str("juggleStartingWeapon", c.juggleStartingWeapon);
    if (const engine::JsonValue *p = params.get("juggleWeapons")) {
        c.juggleWeapons.clear();
        for (const engine::JsonValue &w : p->items) {
            c.juggleWeapons.push_back(w.string);
        }
    }
    if (const engine::JsonValue *p = params.get("includeStartingWeapon")) {
        c.juggleIncludeStarting = p->number != 0 || p->boolean;
    }
    str("upgradeType", c.upgradeType);
    if (const engine::JsonValue *p = params.get("showTally")) {
        c.showTally = p->number != 0 || p->boolean;
    }
    if (const engine::JsonValue *p = params.get("requiredCustomers")) {
        for (const engine::JsonValue &n : p->items) {
            c.requiredCustomerUnlocks.push_back(n.string);
        }
    }
    num("requiredLevel", c.requiredLevelUnlock);
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

// AS: ChallengeManager.getChallengeType
std::string ChallengeManager::getChallengeType(double whichWorld, double whichChallenge) const
{
    if (whichWorld > -1) {
        for (const data::Challenge &c : challenges) {
            if (c.whichWorld == whichWorld && c.whichChallenge == whichChallenge) {
                return c.challengeType;
            }
        }
    }
    return {};
}

// AS: ChallengeManager.getChallengeSkillNeeded
std::string ChallengeManager::getChallengeSkillNeeded(double whichWorld, double whichChallenge) const
{
    if (whichWorld > -1) {
        for (const data::Challenge &c : challenges) {
            if (c.whichWorld == whichWorld && c.whichChallenge == whichChallenge) {
                return c.skillNeeded;
            }
        }
    }
    return {};
}

// AS: ChallengeManager.getChallengeTallyString — "(tally/target)" while a counted challenge is unfinished.
std::string ChallengeManager::getChallengeTallyString(double whichWorld, double whichChallenge) const
{
    std::string text;
    if (whichWorld > -1) {
        for (const data::Challenge &c : challenges) {
            if (c.whichWorld == whichWorld && c.whichChallenge == whichChallenge) {
                if (c.showTally && c.tally < c.targetAmount) {
                    text = "(" + as3::String(c.tally) + "/" + as3::String(c.targetAmount) + ")";
                }
            }
        }
    }
    return text;
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

// Port: a level challenge reached its target: it is completed (once), the HUD updates, the drop-down is queued
// and the custom badges are checked; most record* also save.
void ChallengeManager::completeChallenge(const data::Challenge &c, bool save)
{
    data::UserData &user = *gameObj->obf_51;
    if (user.hasCompletedChallenge(c.whichWorld, c.whichChallenge)) {
        return;
    }
    user.completeChallenge(c.whichWorld, c.whichChallenge);
    if (gameObj->obf_6Z) {
        gameObj->obf_6Z->updateDisplay();
    }
    showCompletingChallenge(c.whichChallenge);
    checkCustomChallenges("challenges");
    if (save) {
        user.saveProgress("challenge");
    }
}

// Port: a badge reached its target: it is earned (once) with its reward.
void ChallengeManager::earnBadge(size_t index, const data::Challenge &b, bool saveFirst)
{
    data::UserData &user = *gameObj->obf_51;
    if (user.hasBadge(static_cast<double>(index))) {
        return;
    }
    user.earnBadge(static_cast<double>(index), b.title);
    if (b.rewardMoney > 0) {
        user.totalMoney += b.rewardMoney;
    }
    if (saveFirst) {
        user.saveProgress("badge");
    }
    if (gameObj->obf_6Z) {
        gameObj->obf_6Z->updateDisplay();
    }
    showEarningBadge(static_cast<double>(index));
    if (!saveFirst) {
        user.saveProgress("badge");
    }
}

// AS: ChallengeManager.recordTag — GameHUD.showTally is empty in the original.
void ChallengeManager::recordTag(const std::string &tag)
{
    const double level = gameObj->obf_AQ->currentLevel;
    for (data::Challenge &c : challenges) {
        if (c.whichWorld == level && c.challengeType == data::Challenge::TAGGED && c.tagName == tag) {
            if (c.addToTally()) {
                completeChallenge(c, true);
            }
        }
    }
    for (size_t i = 0; i < badges.size(); ++i) {
        data::Challenge &b = badges[i];
        if (b.challengeType == data::Challenge::TAGGED && b.tagName == tag && b.addToTally()) {
            earnBadge(i, b);
        }
    }
}

// AS: ChallengeManager.unrecordTag
void ChallengeManager::unrecordTag(const std::string &tag)
{
    const double level = gameObj->obf_AQ->currentLevel;
    for (data::Challenge &c : challenges) {
        if (c.whichWorld == level && c.challengeType == data::Challenge::TAGGED && c.tagName == tag &&
            !gameObj->obf_51->hasCompletedChallenge(c.whichWorld, c.whichChallenge)) {
            c.removeFromTally();
        }
    }
}

// AS: ChallengeManager.recordCustomerCage
void ChallengeManager::recordCustomerCage(double whichCustomer)
{
    const double level = gameObj->obf_AQ->currentLevel;
    for (data::Challenge &c : challenges) {
        if (c.whichWorld == level && c.challengeType == data::Challenge::RESCUE && c.whichCustomer == whichCustomer) {
            completeChallenge(c, false);
        }
    }
}

// AS: ChallengeManager.recordBurgerzilla
void ChallengeManager::recordBurgerzilla(double amount)
{
    SDL_Log("Record Burgerzilla! %s", as3::String(amount).c_str());
    const double level = gameObj->obf_AQ->currentLevel;
    for (data::Challenge &c : challenges) {
        if (c.whichWorld == level && c.challengeType == data::Challenge::BURGERZILLAS) {
            // QUIRK: one Burgerzilla is added whatever `amount` is (addToTally() without the argument).
            const bool done = c.addToTally();
            SDL_Log("Got Challenge. Current Tally = %s/%s.  Completed?: %s", as3::String(c.tally).c_str(),
                    as3::String(c.targetAmount).c_str(), done ? "true" : "false");
            if (done) {
                completeChallenge(c, false);
            }
        }
    }
    for (size_t i = 0; i < badges.size(); ++i) {
        data::Challenge &b = badges[i];
        if (b.challengeType == data::Challenge::BURGERZILLAS && b.addToTally()) {
            earnBadge(i, b);
        }
    }
}

// AS: ChallengeManager.recordCoin — QUIRK: one coin is added whatever `amount` is.
void ChallengeManager::recordCoin(double /*amount*/)
{
    const double level = gameObj->obf_AQ->currentLevel;
    for (data::Challenge &c : challenges) {
        if (c.whichWorld == level && c.challengeType == data::Challenge::COINS && c.addToTally()) {
            completeChallenge(c, false);
        }
    }
    for (size_t i = 0; i < badges.size(); ++i) {
        data::Challenge &b = badges[i];
        if (b.challengeType == data::Challenge::COINS && b.addToTally()) {
            earnBadge(i, b);
        }
    }
}

// AS: ChallengeManager.recordSpecialItem — QUIRK: one item is added whatever `amount` is.
void ChallengeManager::recordSpecialItem(double /*amount*/)
{
    const double level = gameObj->obf_AQ->currentLevel;
    for (data::Challenge &c : challenges) {
        if (c.whichWorld == level && c.challengeType == data::Challenge::SPECIALITEMS && c.addToTally()) {
            completeChallenge(c, false);
        }
    }
    for (size_t i = 0; i < badges.size(); ++i) {
        data::Challenge &b = badges[i];
        if (b.challengeType == data::Challenge::SPECIALITEMS && b.addToTally()) {
            earnBadge(i, b);
        }
    }
}

namespace {

// recordMultiExplosion / recordMultiRocket / recordMultiJuggle / recordCombo: a record of `amount` at once.
void recordBest(std::vector<data::Challenge> &challenges, std::vector<data::Challenge> &badges, double level,
                const char *type, double amount, const std::function<void(data::Challenge &)> &complete,
                const std::function<void(size_t, data::Challenge &)> &earn)
{
    for (data::Challenge &c : challenges) {
        if (c.whichWorld == level && c.challengeType == type && c.targetAmount <= amount && c.setTally(amount)) {
            complete(c);
        }
    }
    for (size_t i = 0; i < badges.size(); ++i) {
        data::Challenge &b = badges[i];
        if (b.challengeType == type && b.targetAmount <= amount && b.setTally(amount)) {
            earn(i, b);
        }
    }
}

} // namespace

// AS: ChallengeManager.recordMultiExplosion
void ChallengeManager::recordMultiExplosion(double amount)
{
    recordBest(challenges, badges, gameObj->obf_AQ->currentLevel, data::Challenge::MULTIEXPLOSION, amount,
               [this](data::Challenge &c) { completeChallenge(c, true); },
               [this](size_t i, data::Challenge &b) { earnBadge(i, b); });
}

// AS: ChallengeManager.recordMultiRocket
void ChallengeManager::recordMultiRocket(double amount)
{
    recordBest(challenges, badges, gameObj->obf_AQ->currentLevel, data::Challenge::MULTIROCKET, amount,
               [this](data::Challenge &c) { completeChallenge(c, true); },
               [this](size_t i, data::Challenge &b) { earnBadge(i, b); });
}

// AS: ChallengeManager.recordMultiJuggle
void ChallengeManager::recordMultiJuggle(double amount)
{
    recordBest(challenges, badges, gameObj->obf_AQ->currentLevel, data::Challenge::MULTIJUGGLE, amount,
               [this](data::Challenge &c) { completeChallenge(c, true); },
               [this](size_t i, data::Challenge &b) { earnBadge(i, b); });
}

// AS: ChallengeManager.recordCombo
void ChallengeManager::recordCombo(double amount)
{
    recordBest(challenges, badges, gameObj->obf_AQ->currentLevel, data::Challenge::COMBO, amount,
               [this](data::Challenge &c) { completeChallenge(c, true); },
               [this](size_t i, data::Challenge &b) { earnBadge(i, b); });
}

// AS: ChallengeManager.recordCompletionTime — a time trial is completed when the level took no longer.
void ChallengeManager::recordCompletionTime(double time)
{
    const double level = gameObj->obf_AQ->currentLevel;
    for (data::Challenge &c : challenges) {
        if (c.whichWorld == level && c.challengeType == data::Challenge::TIMETRIAL && time <= c.targetAmount) {
            completeChallenge(c, true);
        }
    }
    for (size_t i = 0; i < badges.size(); ++i) {
        data::Challenge &b = badges[i];
        if (b.challengeType == data::Challenge::TIMETRIAL && time <= b.targetAmount) {
            earnBadge(i, b);
        }
    }
}

// AS: ChallengeManager.recordEnemyKill
void ChallengeManager::recordEnemyKill(double type, const std::string &weapon, const std::string &category)
{
    const double level = gameObj->obf_AQ->currentLevel;
    for (data::Challenge &c : challenges) {
        if (c.whichWorld == level && c.challengeType == data::Challenge::ENEMYTALLY &&
            c.meetsEnemyRequirements(type, weapon, category) && c.addToTally(1)) {
            completeChallenge(c, true);
        }
    }
    for (size_t i = 0; i < badges.size(); ++i) {
        data::Challenge &b = badges[i];
        if (b.challengeType == data::Challenge::ENEMYTALLY && b.meetsEnemyRequirements(type, weapon, category) &&
            b.addToTally(1)) {
            earnBadge(i, b);
        }
    }
}

// AS: ChallengeManager.recordMastery — SKELETON: ObjectManager.getWeaponType (the weapon's own name is used).
void ChallengeManager::recordMastery(const std::string &weapon)
{
    const double level = gameObj->obf_AQ->currentLevel;
    for (data::Challenge &c : challenges) {
        if (c.whichWorld == level && c.challengeType == data::Challenge::MASTERY &&
            c.meetsMasteryRequirements(weapon) && c.addToTally(1)) {
            completeChallenge(c, true);
        }
    }
    for (size_t i = 0; i < badges.size(); ++i) {
        data::Challenge &b = badges[i];
        if (b.challengeType == data::Challenge::MASTERY && b.meetsMasteryRequirements(weapon) && b.addToTally(1)) {
            earnBadge(i, b);
        }
    }
}

// AS: ChallengeManager.checkCustomChallenges — at the end of a level ("all"): got hit, water, dying, kills; after
// a completed challenge ("challenges"): the badges that count challenges and treasures.
void ChallengeManager::checkCustomChallenges(const std::string &which)
{
    data::UserData &user = *gameObj->obf_51;
    if (which == "all") {
        const double level = gameObj->obf_AQ->currentLevel;
        for (data::Challenge &c : challenges) {
            if (c.whichWorld != level || c.challengeType != data::Challenge::CUSTOM) {
                continue;
            }
            bool done = false;
            if (c.tagName == data::Challenge::CUSTOM_TAG_GOTHIT) {
                done = !user.gotHurt;
            } else if (c.tagName == data::Challenge::CUSTOM_TAG_ENDNOWATER) {
                done = !user.fellInWater;
            } else if (c.tagName == data::Challenge::CUSTOM_TAG_ALLNODYING) {
                done = user.killsTally >= c.targetAmount && user.livesLost == 0;
            } else if (c.tagName == data::Challenge::CUSTOM_TAG_ENDNODYING) {
                done = user.livesLost == 0;
            } else if (c.tagName == data::Challenge::CUSTOM_TAG_NOKILLS) {
                done = user.killsTally <= c.targetAmount;
            }
            if (done) {
                completeChallenge(c, true);
            }
        }
    }
    for (size_t i = 0; i < badges.size(); ++i) {
        data::Challenge &b = badges[i];
        if (b.challengeType != data::Challenge::CUSTOM) {
            continue;
        }
        bool done = false;
        if (b.tagName == data::Challenge::CUSTOM_TAG_CHALLENGES && which == "challenges") {
            done = user.getTotalChallengesCompleted() >= b.targetAmount;
        } else if (b.tagName == data::Challenge::CUSTOM_TAG_TREASURES && which == "challenges") {
            double treasures = 0;
            for (int w = 0; w < 8; ++w) {
                if (user.hasCompletedChallenge(w, 4)) {
                    ++treasures;
                }
            }
            done = b.setTally(treasures);
        } else if (b.tagName == data::Challenge::CUSTOM_TAG_PLAYASCUSTOMERS && which == "all") {
            double used = 0;
            for (double u : user.customersUsed) {
                if (u == 1) {
                    ++used;
                }
            }
            done = b.setTally(used);
        }
        if (done) {
            earnBadge(i, b, false);
        }
    }
}

// AS: ChallengeManager.getChallengeTallyForCheckpoint — the counted challenges' tally, kept by a checkpoint.
double ChallengeManager::getChallengeTallyForCheckpoint(double whichWorld, double whichChallenge) const
{
    double tally = -1;
    if (whichWorld > -1) {
        for (const data::Challenge &c : challenges) {
            if (c.whichWorld == whichWorld && c.whichChallenge == whichChallenge) {
                const std::string &t = c.challengeType;
                if (t == data::Challenge::BURGERZILLAS || t == data::Challenge::COINS ||
                    t == data::Challenge::SPECIALITEMS || t == data::Challenge::TAGGED ||
                    t == data::Challenge::MULTIJUGGLE || t == data::Challenge::MULTIEXPLOSION ||
                    t == data::Challenge::MULTIROCKET || t == data::Challenge::COMBO ||
                    t == data::Challenge::ENEMYTALLY) {
                    tally = c.tally;
                }
            }
        }
    }
    return tally;
}

// AS: ChallengeManager.setChallengeTallyFromCheckpoint
void ChallengeManager::setChallengeTallyFromCheckpoint(double whichWorld, double whichChallenge, double tally)
{
    if (whichWorld > -1) {
        for (data::Challenge &c : challenges) {
            if (c.whichWorld == whichWorld && c.whichChallenge == whichChallenge && tally != -1) {
                c.setTally(tally);
            }
        }
    }
}

// AS: ChallengeManager.resetCurrentChallenges
void ChallengeManager::resetCurrentChallenges()
{
    const double level = gameObj->obf_AQ->currentLevel;
    for (data::Challenge &c : challenges) {
        if (c.whichWorld == level) {
            c.clearTally();
        }
    }
}

// AS: ChallengeManager.showEarningBadge
void ChallengeManager::showEarningBadge(double which)
{
    SDL_Log("EARN BADGE #%s. ADD TO QUEUE!", as3::String(which).c_str());
    badgeQueue.push_back(which);
    checkForDisplayingDropdown();
}

// AS: ChallengeManager.showCompletingChallenge
void ChallengeManager::showCompletingChallenge(double which)
{
    SDL_Log("COMPLETE CHALLENGE #%s. ADD TO QUEUE!", as3::String(which).c_str());
    challengeQueue.push_back(which);
    checkForDisplayingDropdown();
}

// AS: ChallengeManager.showEarningTreasure
void ChallengeManager::showEarningTreasure(double which)
{
    SDL_Log("FOUND TREASURE #%s. ADD TO QUEUE!", as3::String(which).c_str());
    treasureQueue.push_back(which);
    checkForDisplayingDropdown();
}

// AS: ChallengeManager.getChallengeTitle — a level's challenge, or (world -1) badge `whichBadge`.
std::string ChallengeManager::getChallengeTitle(double whichWorld, double whichChallenge, double whichBadge) const
{
    std::string result;
    if (whichWorld > -1) {
        for (const data::Challenge &c : challenges) {
            if (c.whichWorld == whichWorld && c.whichChallenge == whichChallenge) {
                result = c.title;
                break;
            }
        }
    } else if (whichBadge > -1 && whichBadge < static_cast<double>(badges.size())) {
        result = badges[static_cast<size_t>(whichBadge)].title;
    }
    return result;
}

// AS: ChallengeManager.getChallengeDescription
std::string ChallengeManager::getChallengeDescription(double whichWorld, double whichChallenge,
                                                      double whichBadge) const
{
    std::string result;
    if (whichWorld > -1) {
        for (const data::Challenge &c : challenges) {
            if (c.whichWorld == whichWorld && c.whichChallenge == whichChallenge) {
                result = c.description;
                break;
            }
        }
    } else if (whichBadge > -1 && whichBadge < static_cast<double>(badges.size())) {
        result = badges[static_cast<size_t>(whichBadge)].description;
    }
    return result;
}

// AS: ChallengeManager.getChallengeRewardAmount
double ChallengeManager::getChallengeRewardAmount(double whichWorld, double whichChallenge, double whichBadge) const
{
    double result = 0;
    if (whichWorld > -1) {
        for (const data::Challenge &c : challenges) {
            if (c.whichWorld == whichWorld && c.whichChallenge == whichChallenge) {
                result = c.rewardMoney;
                break;
            }
        }
    } else if (whichBadge > -1 && whichBadge < static_cast<double>(badges.size())) {
        result = badges[static_cast<size_t>(whichBadge)].rewardMoney;
    }
    return result;
}

// AS: ChallengeManager.getBadgeTallyString — "(tally/target)" while a counted badge is unearned.
std::string ChallengeManager::getBadgeTallyString(double whichBadge) const
{
    std::string result;
    if (whichBadge < static_cast<double>(badges.size())) {
        const data::Challenge &b = badges[static_cast<size_t>(whichBadge)];
        if (b.showTally && b.tally < b.targetAmount) {
            result = "(" + as3::String(b.tally) + "/" + as3::String(b.targetAmount) + ")";
        }
    }
    return result;
}

// AS: ChallengeManager.getBadgeTally
double ChallengeManager::getBadgeTally(double whichBadge) const
{
    return whichBadge < static_cast<double>(badges.size()) ? badges[static_cast<size_t>(whichBadge)].tally : 0;
}

// AS: ChallengeManager.shouldLockBadge — locked until one of its customers is unlocked, or its area is.
bool ChallengeManager::shouldLockBadge(double whichBadge) const
{
    bool locked = false;
    if (whichBadge < static_cast<double>(badges.size())) {
        const data::Challenge &b = badges[static_cast<size_t>(whichBadge)];
        if (!b.requiredCustomerUnlocks.empty()) {
            locked = true;
            for (const std::string &name : b.requiredCustomerUnlocks) {
                if (gameObj->obf_51->hasCustomerUnlocked(gameObj->obf_3A->getCustomerIndex(name))) {
                    locked = false;
                    break;
                }
            }
        }
        if (b.requiredLevelUnlock > 0) {
            const auto &areas = gameObj->obf_51->areasUnlocked;
            const size_t area = static_cast<size_t>(b.requiredLevelUnlock - 1);
            if (area < areas.size() && areas[area] == 0) {
                locked = true;
            }
        }
    }
    return locked;
}

// AS: ChallengeManager.checkForDisplayingDropdown — with none showing, the first queued treasure, challenge or
// badge drops down from the top of the screen (a badge with its title, description, reward and picture).
// QUIRK: a treasure gets the panel with neither part set up.
void ChallengeManager::checkForDisplayingDropdown()
{
    std::string type = "none";
    double which = -1;
    if (badgeDropdown == nullptr) {
        if (!treasureQueue.empty()) {
            type = "treasure";
            which = treasureQueue.front();
        } else if (!challengeQueue.empty()) {
            type = "challenge";
            which = challengeQueue.front();
        } else if (!badgeQueue.empty()) {
            type = "badge";
            which = badgeQueue.front();
        }
    }
    if (type == "none" || which == -1) {
        return;
    }
    std::unique_ptr<engine::MovieClip> made = engine::MovieClip::create(*gameObj->symbols, "badgeDropdownMC");
    if (!made) {
        return;
    }
    dropdownCurrentType = type;
    badgeDropdown = &gameObj->rootClips->addChild(std::move(made), "");
    badgeDropdown->mouseChildren = false;
    badgeDropdown->setX(static_cast<float>(dropdownX));
    engine::MovieClip *challenge = badgeDropdown->findChild("challenge");
    engine::MovieClip *badge = badgeDropdown->findChild("badge");
    if (type == "badge") {
        if (challenge) {
            challenge->setVisible(false);
        }
        if (badge) {
            badge->setVisible(true);
            if (engine::MovieClip *panel = badge->findChild("panel")) {
                panel->setText("title_txt", getChallengeTitle(-1, -1, which));
                panel->setText("description_txt", getChallengeDescription(-1, -1, which));
                panel->setText("reward_txt", gameObj->lang->get("badge.reward", obf_AF::obf_81::obf_7w(
                                                                       getChallengeRewardAmount(-1, -1, which))));
                panel->setText("points_txt", gameObj->lang->get("badge.points",
                                                                as3::String(getChallengeRewardAmount(-1, -1, which))));
                if (engine::MovieClip *thumb = panel->findChild("thumb")) {
                    thumb->gotoAndStop(static_cast<int>(which) + 1);
                }
            }
        }
        gameObj->obf_7C->playSound("getstar.wav");
    } else if (type == "challenge") {
        if (challenge) {
            challenge->setVisible(true);
            challenge->setText("title_txt", getChallengeTitle(gameObj->obf_AQ->currentLevel, which));
            challenge->setText("number_txt", as3::String(which));
        }
        if (badge) {
            badge->setVisible(false);
        }
        gameObj->obf_7C->playSound("getstar.wav");
    }
    dropdownOnScreenTimer = 0;
    dropdownStartY = 0 - dropdownHeight;
    badgeDropdown->setY(static_cast<float>(dropdownStartY));
    badgeDropdown->onClick = [this] { clickBadgeDropdown(); };
}

// AS: ChallengeManager.clickBadgeDropdown — it goes up on the next frame.
void ChallengeManager::clickBadgeDropdown()
{
    dropdownOnScreenTimer = dropdownOnScreenWait - 1;
}

// AS: ChallengeManager.animateBadgeDropdown (ENTER_FRAME) — it eases down an eighth of the way a frame, waits 150
// frames, eases back up, and the next queued one follows.
void ChallengeManager::animateBadgeDropdown()
{
    if (badgeDropdown == nullptr) {
        return;
    }
    const double y = badgeDropdown->y();
    if (dropdownOnScreenTimer == 0) {
        if (y < dropdownStartY + dropdownHeight) {
            const double dist = dropdownStartY + dropdownHeight - y;
            badgeDropdown->setY(static_cast<float>(y + dist / dropdownSpeed));
            if (dist >= -1 && dist <= 1) {
                badgeDropdown->setY(static_cast<float>(dropdownStartY + dropdownHeight));
                ++dropdownOnScreenTimer;
            }
        }
    } else if (dropdownOnScreenTimer == dropdownOnScreenWait) {
        if (y > dropdownStartY) {
            const double dist = dropdownStartY - y;
            badgeDropdown->setY(static_cast<float>(y + dist / dropdownSpeed));
            if (dist >= -1 && dist <= 1) {
                gameObj->rootClips->removeChild(badgeDropdown);
                badgeDropdown = nullptr;
                auto shift = [](std::vector<double> &q) {
                    if (!q.empty()) {
                        q.erase(q.begin());
                    }
                };
                if (dropdownCurrentType == "treasure") {
                    shift(treasureQueue);
                } else if (dropdownCurrentType == "challenge") {
                    shift(challengeQueue);
                } else if (dropdownCurrentType == "badge") {
                    shift(badgeQueue);
                }
                checkForDisplayingDropdown();
            }
        }
    } else {
        ++dropdownOnScreenTimer;
    }
}

} // namespace papaGame::managers
