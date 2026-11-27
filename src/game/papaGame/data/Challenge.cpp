#include "game/papaGame/data/Challenge.h"

#include <SDL3/SDL.h>

#include "engine/json.h"

namespace papaGame::data {

// AS: Challenge.meetsEnemyRequirements
bool Challenge::meetsEnemyRequirements(double type, const std::string &weapon, const std::string &category) const
{
    return (enemyType == type || enemyType == -1) && (enemyCategory == category || enemyCategory == "any") &&
           (weapon == whichWeapon || whichWeapon == "any");
}

// AS: Challenge.meetsMasteryRequirements
bool Challenge::meetsMasteryRequirements(const std::string &weapon) const
{
    return weaponType == "any" || weaponType == weapon ||
           (weaponType == WEAPON_TYPE_SWING && (weapon == "weapon2" || weapon == "weapon2b" || weapon == "weapon1")) ||
           (weaponType == WEAPON_TYPE_GUNS && (weapon == "pistol" || weapon == "longgun" || weapon == "gatling")) ||
           (weaponType == WEAPON_TYPE_LONGGUN_GATLING && (weapon == "longgun" || weapon == "gatling"));
}

namespace {

bool has(const engine::JsonValue *params, const char *key)
{
    return params && params->get(key) != nullptr;
}

double num(const engine::JsonValue *params, const char *key)
{
    const engine::JsonValue *v = params ? params->get(key) : nullptr;
    return v ? (v->type == engine::JsonValue::Type::Bool ? (v->boolean ? 1 : 0) : v->number) : 0;
}

std::string str(const engine::JsonValue *params, const char *key)
{
    const engine::JsonValue *v = params ? params->get(key) : nullptr;
    return v ? v->string : std::string();
}

bool flag(const engine::JsonValue *params, const char *key)
{
    const engine::JsonValue *v = params ? params->get(key) : nullptr;
    return v && (v->boolean || v->number != 0);
}

} // namespace

// AS: Challenge.Challenge
void Challenge::construct(double world, double challenge, const std::string &type, const engine::JsonValue &p)
{
    whichWorld = world;
    whichChallenge = challenge;
    challengeType = type;
    params = &p;
    if (type == TAGGED) {
        setupTagged();
    } else if (type == TIMETRIAL) {
        setupTimeTrial();
    } else if (type == ENEMYTALLY) {
        setupEnemyTally();
    } else if (type == ENEMYJUGGLE) {
        setupEnemyJuggle();
    } else if (type == COMBO) {
        setupCombo();
    } else if (type == MULTIEXPLOSION) {
        setupMultiExplosion();
    } else if (type == MULTIROCKET) {
        setupMultiRocket();
    } else if (type == MULTIJUGGLE) {
        setupMultiJuggle();
    } else if (type == MASTERY) {
        setupMastery();
    } else if (type == CUSTOM) {
        setupCustom();
    } else if (type == BURGERZILLAS || type == COINS || type == SPECIALITEMS) {
        setupChallenge();
    } else if (type == RESCUE) {
        setupRescue();
    }
    setupTitles();
    params = nullptr;
}

// AS: Challenge.setupTitles
void Challenge::setupTitles()
{
    if (has(params, "title")) {
        title = str(params, "title");
    }
    if (has(params, "description")) {
        description = str(params, "description");
    }
    rewardMoney = has(params, "rewardMoney") ? num(params, "rewardMoney") : 0;
    showTally = has(params, "showTally") && flag(params, "showTally");
    requiredCustomerUnlocks.clear(); // null when absent
    if (const engine::JsonValue *list = params ? params->get("requiredCustomers") : nullptr) {
        for (const engine::JsonValue &n : list->items) {
            requiredCustomerUnlocks.push_back(n.string);
        }
    }
    requiredLevelUnlock = has(params, "requiredLevel") ? num(params, "requiredLevel") : -1;
}

// AS: Challenge.setupChallenge
void Challenge::setupChallenge()
{
    targetAmount = num(params, "targetAmount");
    skillNeeded = str(params, "skillNeeded");
}

// AS: Challenge.setupRescue
void Challenge::setupRescue()
{
    whichCustomer = num(params, "whichCustomer");
    skillNeeded = str(params, "skillNeeded");
}

// AS: Challenge.setupTagged
void Challenge::setupTagged()
{
    tagName = str(params, "tagName");
    targetAmount = num(params, "targetAmount");
}

// AS: Challenge.setupTimeTrial
void Challenge::setupTimeTrial()
{
    targetAmount = num(params, "targetAmount");
}

// AS: Challenge.setupCombo
void Challenge::setupCombo()
{
    targetAmount = num(params, "targetAmount");
}

// AS: Challenge.setupEnemyTally
void Challenge::setupEnemyTally()
{
    if (has(params, "whichWeapon")) {
        whichWeapon = str(params, "whichWeapon");
    }
    if (has(params, "weaponType")) {
        weaponType = str(params, "weaponType");
    }
    if (has(params, "enemyType")) {
        enemyType = num(params, "enemyType");
    }
    if (has(params, "enemyWeapon")) {
        enemyWeapon = str(params, "enemyWeapon");
    }
    if (has(params, "enemyWeaponType")) {
        enemyWeaponType = str(params, "enemyWeaponType");
    }
    if (has(params, "enemyHeadName")) {
        enemyHeadName = str(params, "enemyHeadName");
    } else if (has(params, "enemyHeadType")) {
        enemyHeadName = str(params, "enemyHeadType");
    }
    if (has(params, "enemyCategory")) {
        enemyCategory = str(params, "enemyCategory");
    }
    if (has(params, "targetAmount")) {
        targetAmount = num(params, "targetAmount");
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "### No Target Amount set for Enemy Tally Challenge! (%g,%g)",
                     whichWorld, whichChallenge);
    }
}

// AS: Challenge.setupEnemyJuggle
void Challenge::setupEnemyJuggle()
{
    if (has(params, "juggleTimesHit")) {
        juggleTimesHit = num(params, "juggleTimesHit");
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "### No Juggle Times Amount set for Enemy Juggle Challenge! (%g,%g)",
                     whichWorld, whichChallenge);
    }
    if (has(params, "juggleStartingWeapon")) {
        juggleStartingWeapon = str(params, "juggleStartingWeapon");
    }
    if (has(params, "juggleWeaponsAmount")) {
        juggleWeaponsAmount = num(params, "juggleWeaponsAmount");
    }
    if (const engine::JsonValue *list = params ? params->get("juggleWeapons") : nullptr) {
        juggleWeapons.clear();
        for (const engine::JsonValue &w : list->items) {
            juggleWeapons.push_back(w.string);
        }
    }
    if (has(params, "includeStartingWeapon")) {
        juggleIncludeStarting = flag(params, "includeStartingWeapon");
    }
}

// AS: Challenge.setupMultiExplosion
void Challenge::setupMultiExplosion()
{
    targetAmount = num(params, "targetAmount");
}

// AS: Challenge.setupMultiRocket
void Challenge::setupMultiRocket()
{
    targetAmount = num(params, "targetAmount");
}

// AS: Challenge.setupMultiJuggle
void Challenge::setupMultiJuggle()
{
    targetAmount = num(params, "targetAmount");
}

// AS: Challenge.setupMastery
void Challenge::setupMastery()
{
    if (has(params, "weaponType")) {
        weaponType = str(params, "weaponType");
    }
    if (has(params, "targetAmount")) {
        targetAmount = num(params, "targetAmount");
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "### No Target Amount set for Mastery Challenge! (%g,%g)", whichWorld,
                     whichChallenge);
    }
}

// AS: Challenge.setupCustom
void Challenge::setupCustom()
{
    tagName = str(params, "tagName");
    targetAmount = has(params, "targetAmount") ? num(params, "targetAmount") : 0;
    upgradeType = has(params, "upgradeType") ? str(params, "upgradeType") : "none";
}

} // namespace papaGame::data
