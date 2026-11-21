#pragma once
// AS: papaGame.screens.TrainingPopup — a training bubble (trainingMC at the frame named after the training flag)
// with up to three key caps; in a level it slides down from the top and back up when closed.
#include <string>

#include "engine/movieclip.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::screens {

class TrainingPopup {
public:
    TrainingPopup(obf_9G::obf_4R *gameObj, std::string trainingFlag, bool isInGame, std::string key1 = "",
                  std::string key2 = "", std::string key3 = "", double timeout = -1);

    void setupPopup();
    void updatePopup();
    void closePopup(const std::string *flag = nullptr);
    void removePopup();
    void destroy();

    obf_9G::obf_4R *gameObj = nullptr;
    std::string trainingFlag;
    bool isInGame = false;
    std::string key1, key2, key3;
    double timeout = -1;
    double animTimer = 0;
    double targetY = 0;
    double offscreenY = -75;
    bool isTweeningIn = false;
    bool isTweeningOut = false;
    engine::MovieClip *clip = nullptr; // owned by the game's container

private:
    void setupKey(const char *instance, const std::string &key, const std::string &wordText);
};

} // namespace papaGame::screens
