#pragma once
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
};

} // namespace papaGame::data
