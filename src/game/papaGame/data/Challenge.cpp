#include "game/papaGame/data/Challenge.h"

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

} // namespace papaGame::data
