#pragma once
// AS: papaGame.data.Challenge — one level challenge or badge: its type, what it counts and its tally. Built from
// the `new Challenge(whichWorld, whichChallenge, type, params)` arguments the importer wrote (construct).
#include <string>
#include <vector>

namespace engine { struct JsonValue; }

namespace papaGame::data {

struct Challenge {
    static constexpr const char *TAGGED = "tagged";
    static constexpr const char *COMBO = "combo";
    static constexpr const char *TIMETRIAL = "timetrial";
    static constexpr const char *ENEMYTALLY = "enemytally";
    static constexpr const char *ENEMYJUGGLE = "enemyjuggle";
    static constexpr const char *MULTIJUGGLE = "multijuggle";
    static constexpr const char *MULTIEXPLOSION = "multiexplosion";
    static constexpr const char *MULTIROCKET = "multirocket";
    static constexpr const char *MASTERY = "mastery";
    static constexpr const char *CUSTOM = "custom";
    static constexpr const char *BURGERZILLAS = "burgerzillas";
    static constexpr const char *COINS = "coins";
    static constexpr const char *SPECIALITEMS = "specialitems";
    static constexpr const char *RESCUE = "rescue";
    static constexpr const char *CUSTOM_TAG_GOTHIT = "gotHit";
    static constexpr const char *CUSTOM_TAG_ALLNODYING = "allNoDying";
    static constexpr const char *CUSTOM_TAG_ENDNODYING = "endNoDying";
    static constexpr const char *CUSTOM_TAG_ENDNOWATER = "endNoWater";
    static constexpr const char *CUSTOM_TAG_NOKILLS = "noKills";
    static constexpr const char *CUSTOM_TAG_TREASURES = "gotTreasures";
    static constexpr const char *CUSTOM_TAG_CHALLENGES = "gotChallenges";
    static constexpr const char *CUSTOM_TAG_UPGRADES = "gotUpgrades";
    static constexpr const char *CUSTOM_TAG_ALLWEAPONS = "usedAllWeapons";
    static constexpr const char *CUSTOM_TAG_PLAYASCUSTOMERS = "playAsCustomers";
    static constexpr const char *WEAPON_TYPE_GUNS = "weaponGuns";
    static constexpr const char *WEAPON_TYPE_LONGGUN_GATLING = "weaponLongGatling";
    static constexpr const char *WEAPON_TYPE_SWING = "weaponSwing";
    static constexpr const char *WEAPON_TYPE_PUNCH = "weaponPunch";

    // `new Challenge(whichWorld, whichChallenge, type, params)`: the type picks the setup method; params is the
    // importer's object of named arguments.
    void construct(double world, double challenge, const std::string &type, const engine::JsonValue &params);
    void setupTitles();
    void setupChallenge();
    void setupRescue();
    void setupTagged();
    void setupTimeTrial();
    void setupCombo();
    void setupEnemyTally();
    void setupEnemyJuggle();
    void setupMultiExplosion();
    void setupMultiRocket();
    void setupMultiJuggle();
    void setupMastery();
    void setupCustom();

    // AS: Challenge.clearTally — a level challenge (or any, when forced) starts again from 0.
    void clearTally(bool force = false)
    {
        if (whichWorld != -1 || force) {
            tally = 0;
        }
    }
    // AS: Challenge.setTally / addToTally: true once the target is reached.
    bool setTally(double value)
    {
        tally = value;
        return tally >= targetAmount;
    }
    // AS: Challenge.addToTally
    bool addToTally(double amount = 1)
    {
        tally += amount;
        return tally >= targetAmount;
    }
    // AS: Challenge.removeFromTally
    void removeFromTally(double amount = 1) { tally -= amount; }
    bool meetsEnemyRequirements(double type, const std::string &weapon, const std::string &category) const;
    bool meetsMasteryRequirements(const std::string &weapon) const;
    // AS: Challenge.getRewardAmount
    double getRewardAmount() const { return rewardMoney; }

    std::string title;
    std::string description;
    double rewardMoney = 0;
    double whichWorld = -1;
    double whichChallenge = -1;
    std::string challengeType;
    std::string tagName;
    double targetAmount = 0;
    double tally = 0;
    std::string whichWeapon = "any";
    std::string weaponType = "any";
    double enemyType = -1;
    std::string enemyWeapon = "any";
    std::string enemyWeaponType = "any";
    std::string enemyHeadName = "any";
    std::string enemyCategory = "any";
    double juggleTimesHit = 0;
    double juggleWeaponsAmount = 0;
    std::string juggleStartingWeapon = "any";
    std::vector<std::string> juggleWeapons{"any"};
    bool juggleIncludeStarting = false;
    std::string skillNeeded = "none";
    double whichCustomer = 1;
    std::string upgradeType = "any";
    bool showTally = false;
    std::vector<std::string> requiredCustomerUnlocks; // null in the original when absent
    double requiredLevelUnlock = -1;
    const engine::JsonValue *params = nullptr; // during construct only (the original keeps its params object)
};

} // namespace papaGame::data
