#pragma once
// AS: papaGame.data.Challenge — one level challenge or badge. Ported so far: the fields the HUD and the level
// end read. SKELETON: the tagged / combo / time trial / tally / juggle / mastery set-ups and their checks.
#include <string>

namespace papaGame::data {

struct Challenge {
    static constexpr const char *RESCUE = "rescue";
    static constexpr const char *SPECIALITEMS = "specialitems";
    static constexpr const char *BURGERZILLAS = "burgerzillas";
    static constexpr const char *COINS = "coins";

    std::string title;
    std::string description;
    double rewardMoney = 0;
    double whichWorld = -1;
    double whichChallenge = -1;
    std::string challengeType;
    std::string tagName;
    double targetAmount = 0;
    double tally = 0;
    std::string skillNeeded = "none";
    double whichCustomer = 1;
    bool showTally = false;
};

} // namespace papaGame::data
