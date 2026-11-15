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
    double burgerzillas = 0;
    double killsTally = 0;
    double startingLives = 0;
    double totalLives = 0; // the constructor sets it to startingLives
    double livesLost = 0;
    bool gotHurt = false;
    std::vector<double> enemyKills = std::vector<double>(41, 0);

    // AS: UserData.earnPoints / killBurgerzilla / saveEnemyKill (SKELETON: HUD updateDisplay and challenges)
    void earnPoints(double amount) { points += amount; }
    // AS: UserData.adjustLives — false when no life is left (SKELETON: HUD updateDisplay).
    bool adjustLives(double amount)
    {
        totalLives += amount;
        if (amount < 0) {
            livesLost += -amount;
        }
        return totalLives >= 0;
    }
    // AS: UserData.resetLives
    void resetLives() { totalLives = startingLives; }
    void killBurgerzilla(double amount = 1) { burgerzillas += amount; }
    void saveEnemyKill(double type, const std::string & /*weapon*/ = "none", const std::string & /*category*/ = "")
    {
        ++killsTally;
        if (enemyKills.size() > type) {
            ++enemyKills[static_cast<size_t>(type)];
        }
    }
};

} // namespace papaGame::data
