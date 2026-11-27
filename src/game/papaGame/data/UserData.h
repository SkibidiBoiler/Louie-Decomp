#pragma once
// AS: papaGame.data.UserData — the save slot: the player's progress (areas, scores, challenges, badges,
// customers and outfits, controls) and the tallies of the level being played.
// MochiDigits fields (numbers hidden from memory editors) are plain doubles in the port. The Flipline API calls
// (analytics, "progress saved" notices) are dropped. Saves go to engine::SharedObject files.
#include <memory>
#include <string>
#include <vector>

#include "engine/shared_object.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::data {

// AS: UserData.loadLabelsForSlot's result: what the slot select screen shows for a slot.
struct SlotLabels {
    std::string name;
    double score = 0, coins = 0, warpcoins = 0, customers = 0, character = 0, style = 1, time = 0;
    std::string completion;
    double areasCompleted = 0;
};

class UserData {
public:
    explicit UserData(obf_9G::obf_4R *gameObj);

    // Port: the field initialisers, which the importer writes to userdata.json (the constructor's defaults).
    bool loadTables(const std::string &path);

    void resetLives();
    bool adjustLives(double amount);
    void earnPoints(double amount);
    void earnMoney(double amount = 1);
    void earnWarpCoin();
    void unlockCustomer(double index);
    void collectSpecialItem(double amount = 1);
    void killBurgerzilla(double amount = 1);
    void clearTallies();
    double getTimeBonus() const;
    double getLivesTally() const;
    double getLivesLostTally() const { return livesLost; }
    double getLivesBonus() const;
    double getTreasureBonus() const { return 0; }
    double getChallengeBonus() const;
    double getChallengeTally() const;
    void unlockNextLevel();
    double updateAndGetLevelScore(bool returnThisScore = true, bool save = true);
    // AS: UserData.getCurrentPoints
    double getCurrentPoints() const { return points; }
    // AS: UserData.getCurrentMoney
    double getCurrentMoney() const { return money; }
    // AS: UserData.getCurrentBurgerzillas
    double getCurrentBurgerzillas() const { return burgerzillas; }
    // AS: UserData.getCurrentSpecialItems
    double getCurrentSpecialItems() const { return specialitems; }
    double getLevelHighScore(double level = -1) const;
    void saveHighScore(double score, double level = -1, bool onlyIfBetter = true);
    void setLevelTime(double time);
    double getLevelBestTime(double level = -1) const;
    double getTotalScore() const;
    // AS: UserData.getTotalMoney
    double getTotalMoney() const { return totalMoney; }
    // AS: UserData.getWarpCoins
    double getWarpCoins() const { return warpCoins; }
    double getEnemyKills(double type) const;
    bool hasCompletedChallenge(double world, double challenge) const;
    void completeChallenge(double world, double challenge);
    double getTotalChallengesCompleted() const;
    bool hasBadge(double index) const;
    void earnBadge(double index, const std::string &title = "");
    double getTotalBadgesEarned() const;
    bool hasCustomerUnlocked(double index) const;
    bool hasOutfitUnlocked(double customer, double outfit) const;
    double getBestOutfit(double customer) const;
    bool purchaseOutfit(double customer, double outfit);
    double getTotalCustomersUnlocked() const;
    double getTotalLevelsCompleted() const;
    double hasBeatGame() const;
    // loadSlotDataForBackup: SKELETON (the backup / restore buttons of the slot screen).
    std::unique_ptr<SlotLabels> loadLabelsForSlot(double slot) const;
    bool hasEarnedEverything() const;
    double getCompletionPercentage() const;
    void createNewSlot(double slot, const std::string &name, const std::string &character = "papalouie");
    void loadData(double slot);
    void saveProgress(const std::string &what = "all");
    void eraseSlot(double slot);
    void saveEnemyKill(double type, const std::string &weapon = "none", const std::string &category = "");
    bool setKey(double keyCode, const std::string &action);
    void setupInitialBadgeProgress();
    bool isValidTrainingFlag(const std::string &flag) const;
    bool hasTrained(const std::string &flag) const;
    void setTrained(const std::string &flag);

    obf_9G::obf_4R *gameObj = nullptr;
    std::string saveSlotPrefix = "PapaLouie2_";
    std::string saveSlotVersion = "1.0";
    std::unique_ptr<engine::SharedObject> so;
    double whichSlot = 1;
    std::string playerName = "Marty";
    std::string whichCharacter = "customer";
    double lastCustomerUnlocked = 0;
    double selectedCharacter = 0;
    double selectedStyle = 1;
    double savedCharacterIndex = -1;
    bool savedCharacterWasUnlocked = false;
    double points = 0, money = 0, burgerzillas = 0, specialitems = 0, livesLost = 0, levelTimePlayed = 0;
    double warpCoinsEarned = 0;
    double killsTally = 0;
    bool gotHurt = false;
    bool fellInWater = false;
    bool alreadyEarned100 = false;
    double startingLives = 0;
    double scorePerShortBonus = 0, scorePerLongBonus = 0, scoreShortCutoff = 0, scoreTimeMaximum = 0;
    double scoreLostPerLife = 0, scoreForNoDeaths = 0, scorePerTreasureEarned = 0, scorePerChallengeCompeleted = 0;
    double timeBonus = 0, startingBonus = 0, scoreLostPerMinuteOver = 0;
    std::vector<double> timeBonusMinimums;
    double totalScore = 0, totalMoney = 0, totalLives = 0, warpCoins = 0, totalTimePlayed = 0;
    std::vector<double> areasUnlocked, highScores, bestTimes;
    bool hasRevealedLatestArea = true;
    double lastAreaRevealed = 0;
    std::vector<std::vector<double>> challengesCompleted;
    std::vector<double> customersUnlocked, customersUsed;
    std::vector<std::vector<double>> customerOutfits;
    std::vector<double> enemyKills;
    std::vector<double> medalsEarned, medalProgress;
    bool hasContinuedGame = false;
    bool didClickTwitter = false, didClickFacebook = false;
    std::vector<std::string> trainingFlagNames;
    std::vector<double> trainingFlags;
    double keyCodeLeft = 37, keyCodeRight = 39, keyCodeUp = 38, keyCodeDown = 40, keyCodeJump = 38;
    double keyCodeAttack = 32, keyCodeDrop = 68, keyCodePause = 80;

private:
    void flushSaveSlot();
    void setIndex(std::vector<double> &list, double index, double value);

    double defaultKeyCodeLeft = 37, defaultKeyCodeRight = 39, defaultKeyCodeUp = 38, defaultKeyCodeDown = 40;
    double defaultKeyCodeJump = 38, defaultKeyCodeAttack = 32, defaultKeyCodeDrop = 68, defaultKeyCodePause = 80;
    // Port: the field initialisers, for createNewSlot (which resets the progress to them).
    std::vector<double> initialCustomersUnlocked_, initialAreasUnlocked_, initialHighScores_, initialBestTimes_;
    std::vector<double> initialEnemyKills_, initialCustomersUsed_;
    std::vector<std::vector<double>> initialChallengesCompleted_, initialCustomerOutfits_;
};

} // namespace papaGame::data
