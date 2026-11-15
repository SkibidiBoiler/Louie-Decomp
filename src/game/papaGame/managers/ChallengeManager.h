#pragma once
// AS: papaGame.managers.ChallengeManager (§_-94§ on the game object) — level challenges and badges.
// Ported so far: the challenge table (from challenges.json) and getChallengeTargetAmount.
// SKELETON: recording tags, coins, kills, combos; the drop-down announcements; badges.
#include <string>
#include <vector>

#include "game/papaGame/data/Challenge.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::managers {

class ChallengeManager {
public:
    explicit ChallengeManager(obf_9G::obf_4R *gameObj_) : gameObj(gameObj_) {}

    // Port: the `challenges` and `badges` field initialisers, which the importer writes to challenges.json.
    bool loadTables(const std::string &path);
    std::string getChallengeTargetAmount(double whichWorld, double whichChallenge) const;

    obf_9G::obf_4R *gameObj = nullptr;
    std::vector<data::Challenge> challenges;
    std::vector<data::Challenge> badges;
};

} // namespace papaGame::managers
