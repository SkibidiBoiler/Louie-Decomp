#pragma once
// AS: papaGame.managers.EffectManager (§_-V§ on the game object) — adds, runs and removes effects.
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "engine/sheets.h"
#include "game/papaGame/models/Effect.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::managers {

class EffectManager {
public:
    explicit EffectManager(obf_9G::obf_4R *gameObj);
    ~EffectManager();

    // Port: the effect classes' cycle tables, which the importer writes to effects.json.
    bool loadTables(const std::string &path);

    // Returns the effect's id, or -1 when the class does not exist.
    double addEffect(double x, double y, const std::string &type, const std::string &subtype = "",
                     bool followPlayer = false, double playerXoffset = 0, double playerYoffset = 0);
    void preblitSheets() {} // AS: EffectManager.preblitSheets (empty)
    void processEffects();
    void clearEffects();
    void removeEffect(double effectID, bool nosplice = false);
    models::Effect *getEffect(double effectID) const;

    obf_9G::obf_4R *gameObj = nullptr;
    double counter = 0;
    std::vector<models::Effect *> effects;
    std::vector<std::string> objectTypes;
    std::vector<const engine::Sheet *> objectSheets;

private:
    std::map<double, std::unique_ptr<models::Effect>> byID_;
    std::vector<std::unique_ptr<models::Effect>> graveyard_; // removed effects wait a frame (see EnemyManager)
    std::map<std::string, models::CycleTable> tables_;
};

} // namespace papaGame::managers
