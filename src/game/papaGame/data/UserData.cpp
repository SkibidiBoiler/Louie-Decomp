#include "game/papaGame/data/UserData.h"

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

} // namespace papaGame::data
