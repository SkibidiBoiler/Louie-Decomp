#pragma once
#include <string>
#include <vector>

// AS: papaGame.data.UserData — the save slot (selected character, scores, unlocks, ...).
// SKELETON: only the fields the ported code reads so far.

namespace obf_9G { class obf_4R; }

namespace papaGame::data {

class UserData {
public:
    explicit UserData(obf_9G::obf_4R *gameObj_) : gameObj(gameObj_) {}

    obf_9G::obf_4R *gameObj = nullptr;
    double selectedCharacter = 0; // 0 = Marty, 1 = Rita (new game, see UserData.startNewGame)
    double selectedStyle = 1;
    // MochiDigits in the original (an obfuscated number against memory editing): plain doubles in the port.
    double points = 0;
    double money = 0;
    double specialitems = 0;
    double burgerzillas = 0;
    double killsTally = 0;
    double startingLives = 0;
    double totalLives = 0; // the constructor sets it to startingLives
    double livesLost = 0;
    bool gotHurt = false;
    std::vector<double> customersUnlocked; // from userdata.json (the field's initial array)
    double savedCharacterIndex = -1;
    bool savedCharacterWasUnlocked = false;

    // Port: the class's constant tables, which the importer writes to userdata.json.
    bool loadTables(const std::string &path);
    bool hasCustomerUnlocked(double index) const; // AS: UserData.hasCustomerUnlocked
    void unlockCustomer(double index);            // AS: UserData.unlockCustomer
    std::vector<double> enemyKills = std::vector<double>(41, 0);

    // AS: UserData.earnPoints / killBurgerzilla / saveEnemyKill (SKELETON: HUD updateDisplay and challenges)
    void earnPoints(double amount);
    // AS: UserData.earnMoney / collectSpecialItem (SKELETON: HUD updateDisplay, ChallengeManager.recordSpecialItem)
    void earnMoney(double amount = 1);
    void collectSpecialItem(double amount = 1);
    // AS: UserData.adjustLives — false when no life is left (SKELETON: HUD updateDisplay).
    bool adjustLives(double amount);
    // AS: UserData.resetLives
    void resetLives() { totalLives = startingLives; }
    void killBurgerzilla(double amount = 1);
    void saveEnemyKill(double type, const std::string & /*weapon*/ = "none", const std::string & /*category*/ = "")
    {
        ++killsTally;
        if (enemyKills.size() > type) {
            ++enemyKills[static_cast<size_t>(type)];
        }
    }
};

} // namespace papaGame::data
