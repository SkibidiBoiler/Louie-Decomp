#include "game/papaGame/data/UserData.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>

#include "engine/json.h"
#include "game/as3.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::data {

namespace {

void refreshHUD(obf_9G::obf_4R *gameObj)
{
    if (gameObj->obf_6Z) {
        gameObj->obf_6Z->updateDisplay();
    }
}

double at(const std::vector<double> &list, double index)
{
    return index >= 0 && index < static_cast<double>(list.size()) ? list[static_cast<size_t>(index)] : 0;
}

} // namespace

// AS: UserData.UserData
UserData::UserData(obf_9G::obf_4R *gameObj_) : gameObj(gameObj_)
{
    totalLives = startingLives;
    clearTallies();
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
    // Plain numbers, and MochiDigits written as {"new": "MochiDigits", "args": [value]}.
    auto number = [&](const char *name, double &out) {
        if (const engine::JsonValue *v = json.get(name)) {
            if (v->type == engine::JsonValue::Type::Number) {
                out = v->number;
            } else if (const engine::JsonValue *args = v->get("args"); args && !args->items.empty()) {
                out = args->items[0].number;
            }
        }
    };
    auto list = [&](const char *name, std::vector<double> &out) { out = engine::jsonToArray(json.get(name)); };
    auto grid = [&](const char *name, std::vector<std::vector<double>> &out) {
        out = engine::jsonToArray2(json.get(name));
    };
    number("startingLives", startingLives);
    number("scorePerShortBonus", scorePerShortBonus);
    number("scorePerLongBonus", scorePerLongBonus);
    number("scoreShortCutoff", scoreShortCutoff);
    number("scoreTimeMaximum", scoreTimeMaximum);
    number("scoreLostPerLife", scoreLostPerLife);
    number("scoreForNoDeaths", scoreForNoDeaths);
    number("scorePerTreasureEarned", scorePerTreasureEarned);
    number("scorePerChallengeCompeleted", scorePerChallengeCompeleted);
    number("startingBonus", startingBonus);
    number("scoreLostPerMinuteOver", scoreLostPerMinuteOver);
    number("defaultKeyCodeLeft", defaultKeyCodeLeft);
    number("defaultKeyCodeRight", defaultKeyCodeRight);
    number("defaultKeyCodeUp", defaultKeyCodeUp);
    number("defaultKeyCodeDown", defaultKeyCodeDown);
    number("defaultKeyCodeJump", defaultKeyCodeJump);
    number("defaultKeyCodeAttack", defaultKeyCodeAttack);
    number("defaultKeyCodeDrop", defaultKeyCodeDrop);
    number("defaultKeyCodePause", defaultKeyCodePause);
    list("timeBonusMinimums", timeBonusMinimums);
    list("areasUnlocked", initialAreasUnlocked_);
    list("highScores", initialHighScores_);
    list("bestTimes", initialBestTimes_);
    list("customersUnlocked", initialCustomersUnlocked_);
    list("customersUsed", initialCustomersUsed_);
    list("enemyKills", initialEnemyKills_);
    grid("challengesCompleted", initialChallengesCompleted_);
    grid("customerOutfits", initialCustomerOutfits_);
    trainingFlagNames.clear();
    if (const engine::JsonValue *names = json.get("trainingFlagNames")) {
        for (const engine::JsonValue &n : names->items) {
            trainingFlagNames.push_back(n.string);
        }
    }
    if (const engine::JsonValue *v = json.get("saveSlotPrefix")) {
        saveSlotPrefix = v->string;
    }
    if (const engine::JsonValue *v = json.get("saveSlotVersion")) {
        saveSlotVersion = v->string;
    }
    areasUnlocked = initialAreasUnlocked_;
    highScores = initialHighScores_;
    bestTimes = initialBestTimes_;
    customersUnlocked = initialCustomersUnlocked_;
    customersUsed = initialCustomersUsed_;
    enemyKills = initialEnemyKills_;
    challengesCompleted = initialChallengesCompleted_;
    customerOutfits = initialCustomerOutfits_;
    keyCodeLeft = defaultKeyCodeLeft;
    keyCodeRight = defaultKeyCodeRight;
    keyCodeUp = defaultKeyCodeUp;
    keyCodeDown = defaultKeyCodeDown;
    keyCodeJump = defaultKeyCodeJump;
    keyCodeAttack = defaultKeyCodeAttack;
    keyCodeDrop = defaultKeyCodeDrop;
    keyCodePause = defaultKeyCodePause;
    totalLives = startingLives;
    clearTallies();
    return true;
}

// Port: `list[index] = value`, which grows an AS3 array.
void UserData::setIndex(std::vector<double> &list, double index, double value)
{
    if (index < 0) {
        return;
    }
    const auto i = static_cast<size_t>(index);
    if (i >= list.size()) {
        list.resize(i + 1, 0);
    }
    list[i] = value;
}

// AS: UserData.resetLives
void UserData::resetLives()
{
    totalLives = startingLives;
}

// AS: UserData.adjustLives — false when no life is left.
bool UserData::adjustLives(double amount)
{
    totalLives += amount;
    refreshHUD(gameObj);
    if (amount < 0) {
        livesLost += std::abs(amount);
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

// AS: UserData.earnWarpCoin
void UserData::earnWarpCoin()
{
    SDL_Log("EARN A WARP COIN!");
    gameObj->obf_94->recordTag("warpCoin");
    warpCoins += 1;
    warpCoinsEarned += 1;
}

// AS: UserData.unlockCustomer
void UserData::unlockCustomer(double index)
{
    if (index >= 0 && index < static_cast<double>(customersUnlocked.size())) {
        if (customersUnlocked[static_cast<size_t>(index)] == 0) {
            gameObj->obf_94->recordTag("customerUnlocked");
        }
        customersUnlocked[static_cast<size_t>(index)] = 1;
    }
}

// AS: UserData.collectSpecialItem
void UserData::collectSpecialItem(double amount)
{
    specialitems += amount;
    refreshHUD(gameObj);
    gameObj->obf_94->recordSpecialItem(amount);
}

// AS: UserData.killBurgerzilla
void UserData::killBurgerzilla(double amount)
{
    burgerzillas += amount;
    refreshHUD(gameObj);
    gameObj->obf_94->recordBurgerzilla(amount);
}

// AS: UserData.clearTallies
void UserData::clearTallies()
{
    timeBonus = startingBonus;
    levelTimePlayed = 0;
    livesLost = 0;
    money = 0;
    points = 0;
    burgerzillas = 0;
    specialitems = 0;
    warpCoinsEarned = 0;
    killsTally = 0;
    gotHurt = false;
    fellInWater = false;
    savedCharacterIndex = -1;
}

// AS: UserData.getTimeBonus — the maximum, less a step per minute over the level's par time (frames).
double UserData::getTimeBonus() const
{
    const double level = gameObj->obf_AQ->currentLevel;
    if (totalLives >= 0) {
        const double over = levelTimePlayed - at(timeBonusMinimums, level);
        double steps = 0;
        if (over <= 0) {
            steps = 0;
        } else if (over <= 1800) {
            steps = 1;
        } else if (over <= 3600) {
            steps = 2;
        } else if (over <= 5400) {
            steps = 3;
        } else if (over <= 7200) {
            steps = 4;
        } else if (over > 7200) {
            steps = 5;
        }
        return scoreTimeMaximum - steps * scoreLostPerMinuteOver;
    }
    return 0;
}

// AS: UserData.getLivesTally
double UserData::getLivesTally() const
{
    return totalLives >= 0 ? totalLives : 0;
}

// AS: UserData.getLivesBonus
double UserData::getLivesBonus() const
{
    if (totalLives >= 0) {
        if (livesLost == 0) {
            return scoreForNoDeaths;
        }
        return std::max(0.0, scoreForNoDeaths - livesLost * scoreLostPerLife);
    }
    return 0;
}

// AS: UserData.getChallengeBonus
double UserData::getChallengeBonus() const
{
    return getChallengeTally() * scorePerChallengeCompeleted;
}

// AS: UserData.getChallengeTally — challenges done in the current level.
double UserData::getChallengeTally() const
{
    const double level = gameObj->obf_AQ->currentLevel;
    double count = 0;
    if (level >= 0 && static_cast<double>(challengesCompleted.size()) > level) {
        for (double done : challengesCompleted[static_cast<size_t>(level)]) {
            if (done == 1) {
                ++count;
            }
        }
    }
    return count;
}

// AS: UserData.unlockNextLevel — every world whose warp coin price is paid opens.
void UserData::unlockNextLevel()
{
    const std::vector<double> &prices = gameObj->obf_AQ->coinsToUnlockWorld;
    for (size_t i = 0; i < prices.size(); ++i) {
        if (getWarpCoins() >= prices[i]) {
            if (at(areasUnlocked, static_cast<double>(i)) == 0) {
                SDL_Log(">>>> UNLOCK WORLD %zu", i + 1);
                setIndex(areasUnlocked, static_cast<double>(i), 1);
                hasRevealedLatestArea = false;
            }
        }
    }
}

// AS: UserData.updateAndGetLevelScore
double UserData::updateAndGetLevelScore(bool returnThisScore, bool save)
{
    double score = points + getTimeBonus() + getChallengeBonus();
    if (score < 0) {
        score = 0;
    }
    saveHighScore(score);
    const double result = returnThisScore ? score : getLevelHighScore();
    totalMoney += money;
    totalScore = getTotalScore();
    clearTallies();
    if (save) {
        saveProgress();
    }
    return result;
}

// AS: UserData.getLevelHighScore
double UserData::getLevelHighScore(double level) const
{
    if (level == -1) {
        level = gameObj->obf_AQ->currentLevel;
    }
    return at(highScores, level);
}

// AS: UserData.saveHighScore — the "BeatLevel" analytics event is dropped.
void UserData::saveHighScore(double score, double level, bool onlyIfBetter)
{
    if (level == -1) {
        level = gameObj->obf_AQ->currentLevel;
    }
    if (level >= 0 && static_cast<double>(highScores.size()) > level) {
        const double old = highScores[static_cast<size_t>(level)];
        if (score > old || !onlyIfBetter) {
            highScores[static_cast<size_t>(level)] = score;
        }
    }
}

// AS: UserData.setLevelTime
void UserData::setLevelTime(double time)
{
    const double level = gameObj->obf_AQ->currentLevel;
    if (level >= 0 && static_cast<double>(bestTimes.size()) > level) {
        const double best = bestTimes[static_cast<size_t>(level)];
        if (time < best || best == 0) {
            bestTimes[static_cast<size_t>(level)] = time;
        }
    }
}

// AS: UserData.getLevelBestTime
double UserData::getLevelBestTime(double level) const
{
    if (level == -1) {
        level = gameObj->obf_AQ->currentLevel;
    }
    return at(bestTimes, level);
}

// AS: UserData.getTotalScore — the levels' high scores plus the badges' rewards.
double UserData::getTotalScore() const
{
    double total = 0;
    for (double s : highScores) {
        total += s;
    }
    if (const managers::ChallengeManager *cm = gameObj->obf_94.get()) {
        for (size_t i = 0; i < cm->badges.size(); ++i) {
            if (at(medalsEarned, static_cast<double>(i)) == 1) {
                total += cm->badges[i].rewardMoney;
            }
        }
    }
    return total;
}

// AS: UserData.getEnemyKills
double UserData::getEnemyKills(double type) const
{
    return at(enemyKills, type);
}

// AS: UserData.hasCompletedChallenge (challenge numbers start at 1)
bool UserData::hasCompletedChallenge(double world, double challenge) const
{
    if (world > -1 && static_cast<double>(challengesCompleted.size()) > world) {
        const auto &row = challengesCompleted[static_cast<size_t>(world)];
        return static_cast<double>(row.size()) > challenge - 1 && at(row, challenge - 1) == 1;
    }
    return false;
}

// AS: UserData.completeChallenge — the analytics events are dropped.
void UserData::completeChallenge(double world, double challenge)
{
    if (world >= 0 && static_cast<double>(challengesCompleted.size()) > world) {
        setIndex(challengesCompleted[static_cast<size_t>(world)], challenge - 1, 1);
    }
    if (!alreadyEarned100 && hasEarnedEverything()) {
        alreadyEarned100 = true;
    }
    if (world != 9) {
        earnWarpCoin();
        gameObj->obf_V->addEffect(gameObj->playerObj->x, gameObj->playerObj->y, "WarpCoinEffect", "", true, 0, -30);
    }
}

// AS: UserData.getTotalChallengesCompleted
double UserData::getTotalChallengesCompleted() const
{
    double count = 0;
    for (const auto &row : challengesCompleted) {
        for (double done : row) {
            if (done == 1) {
                ++count;
            }
        }
    }
    return count;
}

// AS: UserData.hasBadge
bool UserData::hasBadge(double index) const
{
    return at(medalsEarned, index) == 1;
}

// AS: UserData.earnBadge — the analytics events are dropped.
void UserData::earnBadge(double index, const std::string & /*title*/)
{
    setIndex(medalsEarned, index, 1);
    if (!alreadyEarned100 && hasEarnedEverything()) {
        alreadyEarned100 = true;
    }
}

// AS: UserData.getTotalBadgesEarned
double UserData::getTotalBadgesEarned() const
{
    return static_cast<double>(std::count(medalsEarned.begin(), medalsEarned.end(), 1.0));
}

// AS: UserData.hasCustomerUnlocked
bool UserData::hasCustomerUnlocked(double index) const
{
    return at(customersUnlocked, index) == 1;
}

// AS: UserData.hasOutfitUnlocked
bool UserData::hasOutfitUnlocked(double customer, double outfit) const
{
    if (customer >= 0 && customer < static_cast<double>(customerOutfits.size()) && outfit >= 0 &&
        outfit < static_cast<double>(customerOutfits[static_cast<size_t>(customer)].size())) {
        return at(customersUnlocked, customer) == 1 && at(customerOutfits[static_cast<size_t>(customer)], outfit) == 1;
    }
    return false;
}

// AS: UserData.getBestOutfit — the last bought outfit (1-based).
double UserData::getBestOutfit(double customer) const
{
    double best = 1;
    if (customer >= 0 && customer < static_cast<double>(customerOutfits.size())) {
        const auto &row = customerOutfits[static_cast<size_t>(customer)];
        for (size_t i = 1; i < row.size(); ++i) {
            if (row[i] == 1) {
                best = static_cast<double>(i) + 1;
            }
        }
    }
    return best;
}

// AS: UserData.purchaseOutfit
bool UserData::purchaseOutfit(double customer, double outfit)
{
    const double price = gameObj->obf_AQ->getOutfitPrice(customer, outfit);
    if (totalMoney >= price) {
        setIndex(customerOutfits[static_cast<size_t>(customer)], outfit, 1);
        totalMoney = totalMoney - price;
        setTrained("styles");
        gameObj->obf_94->recordTag("buyOutfit");
        saveProgress("outfit");
        return true;
    }
    return false;
}

// AS: UserData.getTotalCustomersUnlocked
double UserData::getTotalCustomersUnlocked() const
{
    return static_cast<double>(std::count(customersUnlocked.begin(), customersUnlocked.end(), 1.0));
}

// AS: UserData.getTotalLevelsCompleted
double UserData::getTotalLevelsCompleted() const
{
    double count = 0;
    for (int i = 0; i < 10; ++i) {
        if (at(areasUnlocked, i) == 1 && getLevelHighScore(i) > 0) {
            ++count;
        }
    }
    return count;
}

// AS: UserData.hasBeatGame
double UserData::hasBeatGame() const
{
    return at(areasUnlocked, 8) == 1 && getLevelHighScore(8) > 0 ? 1 : 0;
}

// AS: UserData.loadLabelsForSlot — null for an empty slot.
std::unique_ptr<SlotLabels> UserData::loadLabelsForSlot(double slot) const
{
    const engine::SharedObject saved = engine::SharedObject::getLocal(saveSlotPrefix + as3::String(slot));
    if (!saved.truthy("playerName")) {
        return nullptr;
    }
    auto labels = std::make_unique<SlotLabels>();
    labels->name = saved.get("playerName")->string;
    labels->coins = engine::jsonToNumber(saved.get("totalMoney"));
    labels->warpcoins = engine::jsonToNumber(saved.get("warpCoins"));
    const std::vector<double> unlocked = engine::jsonToArray(saved.get("customersUnlocked"));
    labels->customers = static_cast<double>(std::count(unlocked.begin(), unlocked.end(), 1.0));
    labels->character = saved.truthy("char") ? engine::jsonToNumber(saved.get("char")) : 0;
    labels->style = saved.truthy("style") ? engine::jsonToNumber(saved.get("style")) : 1;
    labels->time = saved.truthy("totalTime") ? engine::jsonToNumber(saved.get("totalTime")) : 0;
    const std::vector<double> scores = engine::jsonToArray(saved.get("highScores"));
    const std::vector<double> medals = engine::jsonToArray(saved.get("medalsEarned"));
    for (double s : scores) {
        labels->score += s;
    }
    if (const managers::ChallengeManager *cm = gameObj->obf_94.get()) {
        for (size_t i = 0; i < cm->badges.size(); ++i) {
            if (at(medals, static_cast<double>(i)) == 1) {
                labels->score += cm->badges[i].rewardMoney;
            }
        }
    }
    double areas = 0, areasDone = 0, challengeCount = 0, challengesDone = 0, badgeCount = 0, badgesDone = 0;
    double outfitCount = 0, outfitsDone = 0;
    for (double s : scores) {
        ++areas;
        if (s > 0) {
            ++areasDone;
        }
    }
    for (const auto &row : engine::jsonToArray2(saved.get("challengesCompleted"))) {
        for (double done : row) {
            ++challengeCount;
            challengesDone += done;
        }
    }
    challengeCount = 49; // QUIRK: the count is replaced by a fixed 49
    if (challengesDone > challengeCount) {
        challengesDone = challengeCount;
    }
    for (double m : medals) {
        if (m == 1) {
            ++badgesDone;
        }
    }
    badgeCount = gameObj->obf_94->getNumberOfBadges();
    const auto outfits = engine::jsonToArray2(saved.get("customerOutfits"));
    for (size_t i = 0; i < outfits.size(); ++i) {
        for (double owned : outfits[i]) {
            ++outfitCount;
            if (at(unlocked, static_cast<double>(i)) == 1) {
                outfitsDone += owned;
            }
        }
    }
    const double areaPart = areasDone / areas * 30;
    const double challengePart = challengesDone / challengeCount * 30;
    const double badgePart = badgeCount > 0 ? badgesDone / badgeCount * 30 : 0;
    const double outfitPart = outfitsDone / outfitCount * 10;
    SDL_Log("Slot %s: Areas: %s/%s. Challenges: %s/%s. Badges: %s/%s. Outfits: %s/%s. ", as3::String(slot).c_str(),
            as3::String(areasDone).c_str(), as3::String(areas).c_str(), as3::String(challengesDone).c_str(),
            as3::String(challengeCount).c_str(), as3::String(badgesDone).c_str(), as3::String(badgeCount).c_str(),
            as3::String(outfitsDone).c_str(), as3::String(outfitCount).c_str());
    labels->completion = as3::String(std::floor(areaPart + challengePart + badgePart + outfitPart));
    labels->areasCompleted = areasDone;
    return labels;
}

// AS: UserData.hasEarnedEverything
bool UserData::hasEarnedEverything() const
{
    return getCompletionPercentage() == 100;
}

// AS: UserData.getCompletionPercentage
double UserData::getCompletionPercentage() const
{
    double areas = 0, areasDone = 0, challengeCount = 0, challengesDone = 0, badgesDone = 0;
    double outfitCount = 0, outfitsDone = 0;
    for (double s : highScores) {
        ++areas;
        if (s > 0) {
            ++areasDone;
        }
    }
    for (const auto &row : challengesCompleted) {
        for (double done : row) {
            ++challengeCount;
            challengesDone += done;
        }
    }
    challengeCount = 49; // QUIRK: as in loadLabelsForSlot
    if (challengesDone > challengeCount) {
        challengesDone = challengeCount;
    }
    for (double m : medalsEarned) {
        if (m == 1) {
            ++badgesDone;
        }
    }
    const double badgeCount = gameObj->obf_94->getNumberOfBadges();
    for (size_t i = 0; i < customerOutfits.size(); ++i) {
        for (double owned : customerOutfits[i]) {
            ++outfitCount;
            if (at(customersUnlocked, static_cast<double>(i)) == 1) {
                outfitsDone += owned;
            }
        }
    }
    const double areaPart = areasDone / areas * 30;
    const double challengePart = challengesDone / challengeCount * 30;
    const double badgePart = badgeCount > 0 ? badgesDone / badgeCount * 30 : 0;
    // QUIRK: the outfit part divides by a variable that is still NaN (_loc15_ / _loc15_), so the sum is NaN and
    // the percentage never reaches 100: hasEarnedEverything is always false.
    (void)outfitsDone;
    (void)outfitCount;
    const double outfitPart = std::nan("");
    return std::floor(areaPart + challengePart + badgePart + outfitPart);
}

// AS: UserData.createNewSlot — a fresh save with the starting progress (the field initialisers).
void UserData::createNewSlot(double slot, const std::string &name, const std::string &character)
{
    gameObj->obf_94->resetAllTallies();
    hasContinuedGame = false;
    whichSlot = slot;
    selectedCharacter = character == "rita" ? 1 : 0;
    selectedStyle = 1;
    totalScore = 0;
    totalMoney = 0;
    totalLives = startingLives;
    totalTimePlayed = 0;
    warpCoins = 1;
    playerName = name;
    whichCharacter = character;
    lastCustomerUnlocked = 0;
    customersUnlocked = initialCustomersUnlocked_;
    customersUsed.assign(initialCustomersUsed_.size(), 0);
    customerOutfits = initialCustomerOutfits_;
    setIndex(customersUsed, selectedCharacter, 1);
    enemyKills.assign(initialEnemyKills_.size(), 0);
    areasUnlocked.assign(initialAreasUnlocked_.size(), 0);
    setIndex(areasUnlocked, 0, 1);
    highScores.assign(initialHighScores_.size(), 0);
    bestTimes.assign(initialBestTimes_.size(), 0);
    hasRevealedLatestArea = false;
    lastAreaRevealed = 0;
    challengesCompleted = initialChallengesCompleted_;
    medalsEarned.clear();
    medalProgress.clear();
    setupInitialBadgeProgress();
    alreadyEarned100 = false;
    didClickFacebook = false;
    didClickTwitter = false;
    trainingFlags.assign(trainingFlagNames.size(), 0);
    keyCodeLeft = defaultKeyCodeLeft;
    keyCodeRight = defaultKeyCodeRight;
    keyCodeUp = defaultKeyCodeUp;
    keyCodeDown = defaultKeyCodeDown;
    keyCodeJump = defaultKeyCodeJump;
    keyCodeAttack = defaultKeyCodeAttack;
    keyCodeDrop = defaultKeyCodeDrop;
    keyCodePause = defaultKeyCodePause;
    so = std::make_unique<engine::SharedObject>(engine::SharedObject::getLocal(saveSlotPrefix + as3::String(slot)));
    saveProgress();
    clearTallies();
    gameObj->obf_AQ->clearCheckpoint();
}

// AS: UserData.loadData — an existing slot, or a new one named "Marty".
void UserData::loadData(double slot)
{
    whichSlot = slot;
    selectedCharacter = 0;
    selectedStyle = 1;
    gameObj->obf_94->resetAllTallies();
    so = std::make_unique<engine::SharedObject>(engine::SharedObject::getLocal(saveSlotPrefix + as3::String(slot)));
    const engine::SharedObject &d = *so;
    if (d.truthy("playerName")) {
        SDL_Log("Load from Existing Slot %s", as3::String(slot).c_str());
        playerName = d.get("playerName")->string;
        whichCharacter = d.get("whichCharacter") ? d.get("whichCharacter")->string : std::string();
        totalScore = engine::jsonToNumber(d.get("totalScore"));
        totalMoney = engine::jsonToNumber(d.get("totalMoney"));
        totalLives = engine::jsonToNumber(d.get("totalLives"));
        warpCoins = d.truthy("warpCoins") ? engine::jsonToNumber(d.get("warpCoins")) : 1;
        areasUnlocked = engine::jsonToArray(d.get("areasUnlocked"));
        highScores = engine::jsonToArray(d.get("highScores"));
        challengesCompleted = engine::jsonToArray2(d.get("challengesCompleted"));
        medalsEarned = engine::jsonToArray(d.get("medalsEarned"));
        hasRevealedLatestArea = d.truthy("revealedarea");
        if (d.truthy("lastarearevealed")) {
            lastAreaRevealed = engine::jsonToNumber(d.get("lastarearevealed"));
        } else {
            for (int i = 0; i < 10; ++i) {
                if (at(areasUnlocked, i) == 1) {
                    lastAreaRevealed = i;
                }
            }
        }
        if (d.truthy("enemyKills")) {
            enemyKills = engine::jsonToArray(d.get("enemyKills"));
        } else {
            enemyKills.assign(44, 0); // QUIRK: 44 here, 41 in a new slot
        }
        customersUnlocked = engine::jsonToArray(d.get("customersUnlocked"));
        customerOutfits = engine::jsonToArray2(d.get("customerOutfits"));
        lastCustomerUnlocked = engine::jsonToNumber(d.get("lastCustomerUnlocked"));
        if (d.truthy("customersUsed")) {
            customersUsed = engine::jsonToArray(d.get("customersUsed"));
        } else {
            customersUsed = initialCustomersUsed_;
        }
        selectedCharacter = d.truthy("char") ? engine::jsonToNumber(d.get("char")) : 0;
        selectedStyle = d.truthy("style") ? engine::jsonToNumber(d.get("style")) : 1;
        if (d.truthy("bestTimes")) {
            bestTimes = engine::jsonToArray(d.get("bestTimes"));
        } else {
            bestTimes.assign(initialBestTimes_.size(), 0);
        }
        medalProgress = d.truthy("medalProgress") ? engine::jsonToArray(d.get("medalProgress")) : std::vector<double>{};
        if (totalLives < 0) {
            totalLives = startingLives;
        }
        didClickTwitter = d.truthy("clicktwitter");
        didClickFacebook = d.truthy("clickfacebook");
        if (d.truthy("trainingflags")) {
            trainingFlags = engine::jsonToArray(d.get("trainingflags"));
            if (trainingFlags.size() < trainingFlagNames.size()) {
                trainingFlags.resize(trainingFlagNames.size(), 0);
            }
        } else {
            trainingFlags.assign(trainingFlagNames.size(), 0);
        }
        if (d.truthy("keyCodeLeft")) {
            keyCodeLeft = engine::jsonToNumber(d.get("keyCodeLeft"));
            keyCodeRight = engine::jsonToNumber(d.get("keyCodeRight"));
            keyCodeUp = engine::jsonToNumber(d.get("keyCodeUp"));
            keyCodeDown = engine::jsonToNumber(d.get("keyCodeDown"));
            keyCodeJump = engine::jsonToNumber(d.get("keyCodeJump"));
            keyCodeAttack = engine::jsonToNumber(d.get("keyCodeAttack"));
            keyCodeDrop = engine::jsonToNumber(d.get("keyCodeDrop"));
        } else {
            keyCodeLeft = defaultKeyCodeLeft;
            keyCodeRight = defaultKeyCodeRight;
            keyCodeUp = defaultKeyCodeUp;
            keyCodeDown = defaultKeyCodeDown;
            keyCodeJump = defaultKeyCodeJump;
            keyCodeAttack = defaultKeyCodeAttack;
            keyCodeDrop = defaultKeyCodeDrop;
        }
        keyCodePause = d.truthy("keyCodePause") ? engine::jsonToNumber(d.get("keyCodePause")) : defaultKeyCodePause;
        totalTimePlayed = d.truthy("totalTime") ? engine::jsonToNumber(d.get("totalTime")) : 0;
        gameObj->obf_94->populateMedalProgress(medalProgress);
        alreadyEarned100 = hasEarnedEverything();
        hasContinuedGame = true; // both branches of the original set it (they differ only in analytics)
    } else {
        createNewSlot(slot, "Marty");
    }
    clearTallies();
    gameObj->obf_AQ->clearCheckpoint();
    gameObj->obf_94->fixBadges();
}

// AS: UserData.saveProgress — `what` picks the fields: all, quitlevel, time, badge, outfit, challenge, controls.
void UserData::saveProgress(const std::string &what)
{
    const Uint64 start = SDL_GetTicks();
    if (!so) {
        SDL_Log("Saved data was missing for this save.  Generating a new one.");
        so = std::make_unique<engine::SharedObject>(
            engine::SharedObject::getLocal(saveSlotPrefix + as3::String(whichSlot)));
    }
    engine::SharedObject &d = *so;
    using engine::jsonArray;
    using engine::jsonArray2;
    using engine::jsonBool;
    using engine::jsonNumber;
    using engine::jsonString;
    auto controls = [&] {
        d.set("keyCodeLeft", jsonNumber(keyCodeLeft));
        d.set("keyCodeRight", jsonNumber(keyCodeRight));
        d.set("keyCodeUp", jsonNumber(keyCodeUp));
        d.set("keyCodeDown", jsonNumber(keyCodeDown));
        d.set("keyCodeJump", jsonNumber(keyCodeJump));
        d.set("keyCodeAttack", jsonNumber(keyCodeAttack));
        d.set("keyCodeDrop", jsonNumber(keyCodeDrop));
        d.set("keyCodePause", jsonNumber(keyCodePause));
    };
    auto medals = [&] {
        d.set("medalsEarned", jsonArray(medalsEarned));
        d.set("medalProgress", jsonArray(gameObj->obf_94->getMedalProgressArray()));
    };
    auto social = [&] {
        d.set("clicktwitter", jsonBool(didClickTwitter));
        d.set("clickfacebook", jsonBool(didClickFacebook));
    };
    if (what == "all" || what == "quitlevel") {
        d.set("playerName", jsonString(playerName));
        d.set("whichCharacter", jsonString(whichCharacter));
        d.set("continuedgame", jsonBool(hasContinuedGame));
        d.set("totalScore", jsonNumber(totalScore));
        d.set("totalMoney", jsonNumber(totalMoney));
        d.set("totalLives", jsonNumber(totalLives));
        d.set("totalTime", jsonNumber(totalTimePlayed));
        d.set("warpCoins", jsonNumber(warpCoins));
        d.set("challengesCompleted", jsonArray2(challengesCompleted));
        medals();
        d.set("revealedarea", jsonBool(hasRevealedLatestArea));
        d.set("lastarearevealed", jsonNumber(lastAreaRevealed));
        d.set("customersUnlocked", jsonArray(customersUnlocked));
        d.set("customerOutfits", jsonArray2(customerOutfits));
        d.set("lastCustomerUnlocked", jsonNumber(lastCustomerUnlocked));
        d.set("enemyKills", jsonArray(enemyKills));
        d.set("customersUsed", jsonArray(customersUsed));
        d.set("char", jsonNumber(selectedCharacter));
        d.set("style", jsonNumber(selectedStyle));
        social();
        if (what == "all") {
            d.set("areasUnlocked", jsonArray(areasUnlocked));
            d.set("highScores", jsonArray(highScores));
            d.set("bestTimes", jsonArray(bestTimes));
            controls();
        }
    } else if (what == "time") {
        d.set("totalTime", jsonNumber(totalTimePlayed));
        d.set("continuedgame", jsonBool(hasContinuedGame));
    } else if (what == "badge") {
        medals();
        d.set("totalMoney", jsonNumber(totalMoney));
        d.set("continuedgame", jsonBool(hasContinuedGame));
        social();
    } else if (what == "outfit") {
        d.set("customersUnlocked", jsonArray(customersUnlocked));
        d.set("customerOutfits", jsonArray2(customerOutfits));
        d.set("totalMoney", jsonNumber(totalMoney));
        social();
        medals();
    } else if (what == "challenge") {
        d.set("challengesCompleted", jsonArray2(challengesCompleted));
        d.set("continuedgame", jsonBool(hasContinuedGame));
        medals();
        social();
    } else if (what == "controls") {
        controls();
        d.set("continuedgame", jsonBool(hasContinuedGame));
    }
    d.set("version", jsonString(saveSlotVersion));
    d.set("trainingflags", jsonArray(trainingFlags));
    flushSaveSlot();
    SDL_Log("Saving (%s): %llu ms.", what.c_str(), static_cast<unsigned long long>(SDL_GetTicks() - start));
}

// AS: UserData.eraseSlot
void UserData::eraseSlot(double slot)
{
    engine::SharedObject saved = engine::SharedObject::getLocal(saveSlotPrefix + as3::String(slot));
    saved.clear();
}

// AS: UserData.flushSaveSlot — Flash's "allow more storage" prompts have no counterpart.
void UserData::flushSaveSlot()
{
    if (so) {
        if (so->flush()) {
            SDL_Log("Saved.");
        } else {
            SDL_Log("Error saving to Save Slot, may be disabled from saving.");
        }
    } else {
        SDL_Log("Trying to Flush/Save, but there's no save slot!");
    }
}

// AS: UserData.saveEnemyKill
void UserData::saveEnemyKill(double type, const std::string &weapon, const std::string &category)
{
    ++killsTally;
    if (static_cast<double>(enemyKills.size()) > type && type >= 0) {
        ++enemyKills[static_cast<size_t>(type)];
    }
    gameObj->obf_94->recordEnemyKill(type, weapon, category);
}

// AS: UserData.setKey — false when the key is already used by another action.
bool UserData::setKey(double keyCode, const std::string &action)
{
    bool taken = false;
    if (keyCodeAttack == keyCode && action != "attack") {
        taken = true;
    } else if (keyCodeJump == keyCode && action != "jump" && action != "up") {
        taken = true;
    } else if (keyCodeDown == keyCode && action != "down") {
        taken = true;
    } else if (keyCodeUp == keyCode && action != "up" && action != "jump") {
        taken = true;
    } else if (keyCodeLeft == keyCode && action != "left") {
        taken = true;
    } else if (keyCodeRight == keyCode && action != "right") {
        taken = true;
    }
    if (taken) {
        return false;
    }
    if (action == "attack") {
        keyCodeAttack = keyCode;
    } else if (action == "jump") {
        keyCodeJump = keyCode;
    } else if (action == "drop") {
        keyCodeDrop = keyCode;
    } else if (action == "down") {
        keyCodeDown = keyCode;
    } else if (action == "up") {
        keyCodeUp = keyCode;
    } else if (action == "left") {
        keyCodeLeft = keyCode;
    } else if (action == "right") {
        keyCodeRight = keyCode;
    } else if (action == "pause") {
        keyCodePause = keyCode;
    }
    saveProgress("controls");
    return true;
}

// AS: UserData.setupInitialBadgeProgress
void UserData::setupInitialBadgeProgress()
{
    managers::ChallengeManager &cm = *gameObj->obf_94;
    for (size_t i = 0; i < cm.badges.size(); ++i) {
        medalsEarned.push_back(0);
        medalProgress.push_back(0);
    }
    cm.recordTag("warpCoin");
    cm.recordTag("customerUnlocked");
    cm.recordTag("customerUnlocked");
}

// AS: UserData.isValidTrainingFlag
bool UserData::isValidTrainingFlag(const std::string &flag) const
{
    return std::find(trainingFlagNames.begin(), trainingFlagNames.end(), flag) != trainingFlagNames.end();
}

// AS: UserData.hasTrained
bool UserData::hasTrained(const std::string &flag) const
{
    const auto it = std::find(trainingFlagNames.begin(), trainingFlagNames.end(), flag);
    if (it == trainingFlagNames.end()) {
        return false;
    }
    return at(trainingFlags, static_cast<double>(it - trainingFlagNames.begin())) == 1;
}

// AS: UserData.setTrained
void UserData::setTrained(const std::string &flag)
{
    const auto it = std::find(trainingFlagNames.begin(), trainingFlagNames.end(), flag);
    if (it != trainingFlagNames.end()) {
        setIndex(trainingFlags, static_cast<double>(it - trainingFlagNames.begin()), 1);
    }
}

} // namespace papaGame::data
