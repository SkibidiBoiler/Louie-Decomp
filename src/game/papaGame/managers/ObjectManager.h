#pragma once
// AS: papaGame.managers.ObjectManager (§_-7u§ on the game object) — builds, runs and removes the room's objects.
// SKELETON: carried objects between rooms, events (setEvent*), training signs and checkpoint saves.
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "engine/sheets.h"
#include "game/as3.h"
#include "game/papaGame/models/GameObject.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::managers {

class ObjectManager {
public:
    explicit ObjectManager(obf_9G::obf_4R *gameObj);
    ~ObjectManager();

    // Port: the base GameObject's constants, which the importer writes to objects.json.
    bool loadTables(const std::string &path);

    void updateTrainingSigns();
    models::GameObject *getObject(double screenObjID, double whichScreen) const;
    void stopCameraPan(double screenObjID);
    std::string getWeaponType(const std::string &weapon) const;
    void checkSteppingOnObjects(models::Character &c, bool byObject = false);
    void releaseStandingChars(models::GameObject &o); // port: no dangling whichMovingTile
    double addObject(double xtile, double ytile, double type, double whichScreen, double screenObjID,
                     const std::vector<as3::Value> &extraParams = {});
    void processObjects();
    void clearObjects(double whichScreen, bool all = false, bool resetCounter = true, bool keepCarried = false);
    void destroy() { clearObjects(1, true, false, true); } // AS: ObjectManager.destroy
    void saveObjects();
    void saveObjectsAtCheckpoint();
    void buildObjects(double whichScreen);
    void removeObject(double objectID, bool nosplice = false);
    void preblitSheets() {} // AS: ObjectManager.preblitSheets (empty)
    void activateObject(double whichScreen, double screenObjID, bool pan, double amount, bool fromGlobal);
    void setEvent(double whichScreen, double screenObjID, const std::vector<std::vector<double>> &triggers, double pan,
                  double panTarget);
    void setEventTriggers(double whichScreen, double screenObjID, double necessary,
                          const std::vector<std::vector<double>> &from = {});

    obf_9G::obf_4R *gameObj = nullptr;
    double lastBuildDuration = 0;
    double counter = 0;
    double maxScreenID = 0;
    std::vector<models::GameObject *> objects; // null where removeObject(id, true) left a hole
    std::vector<double> objectTypes;
    std::vector<const engine::Sheet *> objectSheets;

private:
    // Port: owns the objects; removed ones wait a frame (see EnemyManager).
    std::map<double, std::unique_ptr<models::GameObject>> byID_;
    std::vector<std::unique_ptr<models::GameObject>> graveyard_;
};

} // namespace papaGame::managers
