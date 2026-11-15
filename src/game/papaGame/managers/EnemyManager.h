#pragma once
// AS: papaGame.managers.EnemyManager (§_-E§ on the game object) — builds, runs and removes the room's enemies.
// SKELETON: event triggers (activateObject, setEvent*), group messages, camera pans and weapon sheets.
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "engine/sheets.h"
#include "game/as3.h"
#include "game/papaGame/models/Enemy.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::managers {

class EnemyManager {
public:
    explicit EnemyManager(obf_9G::obf_4R *gameObj);
    ~EnemyManager();

    // Port: the enemy classes' constants (cycle tables), which the importer writes to enemies.json.
    bool loadTables(const std::string &path);

    double addEnemy(double xtile, double ytile, double type, double whichScreen, double whichTileset,
                    double screenObjID, const std::vector<as3::Value> &extraParams = {});
    void processEnemies();
    void clearEnemies(double whichScreen, bool all = false, bool resetCounter = true);
    void saveEnemies();
    void buildEnemies(double whichScreen);
    void removeEnemy(double enemyID, bool nosplice = false);
    models::Enemy *getEnemy(double screenObjID, double whichScreen) const;
    void addToPreblit(double type, const std::string &sheetName);
    void preblitSheets();

    obf_9G::obf_4R *gameObj = nullptr;
    double lastBuildDuration = 0;
    double counter = 0;
    std::vector<models::Enemy *> enemies; // null where removeEnemy(id, true) left a hole (the original's 0)
    std::vector<std::string> preblitSheetNames;
    std::vector<double> preblitObjectTypes;
    std::vector<double> objectTypes;
    std::vector<const engine::Sheet *> objectSheets;
    double jugglingEnemies = 0;

private:
    // Port: owns the enemies (`manager["Enemy" + id]`). Removed enemies wait in graveyard_ until the next
    // frame, because the original removes them from inside their own updateObject and keeps running it.
    std::map<double, std::unique_ptr<models::Enemy>> byID_;
    std::vector<std::unique_ptr<models::Enemy>> graveyard_;
    std::map<std::string, models::CycleTable> tables_; // by class name, "Enemy1".."Enemy39"
};

} // namespace papaGame::managers
