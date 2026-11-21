#pragma once
// AS: papaGame.screens.GameOutroScreen — the ending picture: every unlocked customer posed in three rows around Papa
// Louie; after 250 frames an iris closes it into the day's tally.
#include <string>
#include <vector>

#include "engine/movieclip.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::screens {

class GameOutroScreen {
public:
    explicit GameOutroScreen(obf_9G::obf_4R *gameObj);

    void setupScreen();
    void updateScreen();
    void finishGameOutro();
    void destroy();
    void setupModels();
    std::unique_ptr<engine::MovieClip> buildCustomer(double index);
    void clearCustomer(engine::MovieClip *customer);

    obf_9G::obf_4R *gameObj = nullptr;
    engine::MovieClip *clip = nullptr; // owned by the game's container
    double currentPage = 1;
    double totalPages = 3;
    bool didSkip = false;
    std::vector<std::string> frontRowNames{"Georgito", "Penny", "Marty", "Rita", "Greg", "Yippy", "Papa Louie"};
    std::vector<double> frontRowX{-14.5, 88, 192, 507, 608, 715, 275};
    std::vector<double> frontRowY{204, 206, 216, 216, 206, 206, 216};
    double frontRowScale = 0.82;
    std::vector<std::string> middleRowNames{"Prudence", "Clover", "Sarge Fan", "Mindy",   "Ninjoy",
                                            "Boomer",   "Captain Cori", "Xandra", "Taylor", "Scooter"};
    std::vector<double> middleRowX{-9, 57, 116, 179, 250, 464, 517, 569, 657, 703};
    std::vector<double> middleRowY{142, 141, 143, 142, 142, 142, 142, 142, 142, 143, 143};
    double middleRowScale = 0.76;
    std::vector<std::string> backRowNames{"Peggy",    "Akari",    "James", "Professor Fitz", "Kahuna", "Big Pauly",
                                          "Kingsley", "Rico",     "Connor", "Foodini",       "Zoe"};
    std::vector<double> backRowX{2, 49, 100, 171, 216, 285, 429, 500, 563, 617, 692};
    std::vector<double> backRowY{71, 70, 69, 69, 69, 69, 69, 69, 70, 70, 71};
    double backRowScale = 0.67;
    double outroTimer = 0;
    double outroTimerMax = 250;
    std::vector<engine::MovieClip *> models; // owned by the row holders

private:
    // Port: one row of setupModels (the original repeats the loop three times).
    void setupRow(const std::vector<std::string> &names, const std::vector<double> &xs, const std::vector<double> &ys,
                  double scale, const char *holderName, int row);
};

} // namespace papaGame::screens
