#pragma once
// AS: papaGame.managers.ChallengeManager (§_-94§ on the game object) — level challenges and badges.
// What the player does is recorded here: each level challenge or badge it completes is saved and announced by a
// drop-down. SKELETON: juggles.
#include <string>
#include <vector>

#include "engine/movieclip.h"

#include "game/papaGame/data/Challenge.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::managers {

class ChallengeManager {
public:
    explicit ChallengeManager(obf_9G::obf_4R *gameObj_) : gameObj(gameObj_) {}

    // Port: the `challenges` and `badges` field initialisers, which the importer writes to challenges.json.
    bool loadTables(const std::string &path);
    std::string getChallengeTargetAmount(double whichWorld, double whichChallenge) const;
    std::string getChallengeType(double whichWorld, double whichChallenge) const;        // AS
    std::string getChallengeSkillNeeded(double whichWorld, double whichChallenge) const; // AS
    std::string getChallengeTallyString(double whichWorld, double whichChallenge) const; // AS
    double getNumberOfBadges() const { return static_cast<double>(badges.size()); } // AS
    std::vector<double> getMedalProgressArray() const;        // AS: ChallengeManager.getMedalProgressArray
    void populateMedalProgress(const std::vector<double> &progress); // AS: ChallengeManager.populateMedalProgress
    void resetAllTallies();                                   // AS: ChallengeManager.resetAllTallies
    void fixBadges() {}                                       // AS: ChallengeManager.fixBadges (empty)
    void recordTag(const std::string &tag);
    void unrecordTag(const std::string &tag);
    void recordCustomerCage(double whichCustomer);
    void recordBurgerzilla(double amount = 1);
    void recordCoin(double amount = 1);
    void recordSpecialItem(double amount = 1);
    void recordMultiExplosion(double amount);
    void recordMultiRocket(double amount);
    void recordMultiJuggle(double amount);
    void recordCombo(double amount);
    void recordCompletionTime(double time);
    void recordEnemyKill(double type, const std::string &weapon, const std::string &category);
    void recordMastery(const std::string &weapon);
    void checkCustomChallenges(const std::string &which = "all");
    double getChallengeTallyForCheckpoint(double whichWorld, double whichChallenge) const;
    void setChallengeTallyFromCheckpoint(double whichWorld, double whichChallenge, double tally);
    void resetCurrentChallenges();
    void showEarningBadge(double which);
    void showCompletingChallenge(double which);
    void showEarningTreasure(double which);
    std::string getChallengeTitle(double whichWorld, double whichChallenge, double whichBadge = -1) const;
    std::string getChallengeDescription(double whichWorld, double whichChallenge, double whichBadge = -1) const;
    double getChallengeRewardAmount(double whichWorld, double whichChallenge, double whichBadge = -1) const;
    std::string getBadgeTallyString(double whichBadge) const;
    double getBadgeTally(double whichBadge) const;
    bool shouldLockBadge(double whichBadge) const;
    void checkForDisplayingDropdown();
    void clickBadgeDropdown();
    void animateBadgeDropdown(); // the original's ENTER_FRAME on the drop-down; obf_4R calls it every frame

    obf_9G::obf_4R *gameObj = nullptr;
    std::vector<data::Challenge> challenges;
    std::vector<data::Challenge> badges;
    std::vector<double> badgeQueue, challengeQueue, treasureQueue;
    engine::MovieClip *badgeDropdown = nullptr; // owned by gameObj->rootClips
    double dropdownX = 188;
    double dropdownStartY = 0;
    double dropdownHeight = 81;
    double dropdownSpeed = 8;
    double dropdownOnScreenWait = 150;
    double dropdownOnScreenTimer = 0;
    std::string dropdownCurrentType;

private:
    // Port: the blocks every record* repeats for a completed level challenge and an earned badge.
    void completeChallenge(const data::Challenge &c, bool save);
    void earnBadge(size_t index, const data::Challenge &b, bool saveFirst = true);
};

} // namespace papaGame::managers
