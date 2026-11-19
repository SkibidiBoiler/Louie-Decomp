#pragma once
// AS: papaGame.screens.ScoreTallyScreen — LEVEL COMPLETE (the API's EndOfDay): the level's points, time bonus,
// challenge bonus, level score, high score, total and coins are counted up one after another, the level's
// challenges are shown, and the customer lands among three unlocked customers who cheer. Getting the values also
// records the level (unlocks the next one, keeps the time and the high score).
// The leaderboard (Submit Score) went to an online service and is not shown.
#include <memory>
#include <string>

#include "engine/movieclip.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::screens {

class ScoreTallyScreen {
public:
    ScoreTallyScreen(obf_9G::obf_4R *gameObj, engine::MovieClip *container);

    void setupScreen();
    void getScoreValues();
    void nextTally();
    void tallyScore();
    void updateScreen();
    bool checkAndIncrement();
    void scoreTallyClosed();
    void clickContinue();
    void clickQuit();
    void setupCharacters(bool setup = true);
    void updateCharacters();
    void destroy();

    obf_9G::obf_4R *gameObj = nullptr;
    engine::MovieClip *irisClip = nullptr;  // owned by the container
    engine::MovieClip *clip = nullptr;      // owned by the container
    engine::MovieClip *container = nullptr;
    double speed = 32;
    double screenHeight = 480;
    bool animatingIn = false, animatingOut = false;
    double delayBetweenReveals = 15;
    double delayTimer = -20;
    std::string revealingWhich = "points";
    bool fromGameOver = false;
    std::string endAction = "none";
    bool isNewHighScore = false;
    double tally_points = 0, tally_timepoints = 0, tally_challenge = 0, tally_levelpoints = 0;
    double tally_high = 0, tally_high_previous = 0, tally_totalpoints = 0, tally_total_previous = 0;
    double tally_money = 0, tally_totalmoney = 0, tally_totalmoney_previous = 0, tally_keys = 0;
    double whichTallyIndex = 0;
    static constexpr const char *tallyLabels[] = {"points", "timepoints", "challenge", "levelpoints",
                                                  "high",   "totalpoints", "money",    "done"};
    engine::MovieClip *crowdChar1 = nullptr, *crowdChar2 = nullptr, *crowdChar3 = nullptr; // owned by charholder
    engine::MovieClip *playerChar = nullptr, *unlockedChar = nullptr;
    bool unlockedJumping = true, playerJumping = true, playerLanded = false, unlockedLanded = false;
    double unlockedStartX = 192, unlockedTargetX = 172, unlockedStartY = 89, unlockedTargetY = 159;
    double playerStartX = 320, playerTargetX = 340, playerStartY = 89, playerTargetY = 159;
    double unlockedJumpSpeed = -12, playerJumpSpeed = -16, walkSpeed = 1;
    bool didUnlockNewCustomer = false;
    double unlockedNewCustomerIndex = -1;

private:
    std::unique_ptr<engine::MovieClip> buildModel(double whichCharacter, double style);
    double &tallyValue(const std::string &label); // `this["tally_" + label]`
};

} // namespace papaGame::screens
