#pragma once
// AS: papaGame.managers.BulletManager (§_-4l§ on the game object) — adds, runs and removes bullets; one sheet per
// bullet type.
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "engine/sheets.h"
#include "game/papaGame/models/Bullet.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::managers {

class BulletManager {
public:
    explicit BulletManager(obf_9G::obf_4R *gameObj);
    ~BulletManager();

    // Port: the bullet classes' cycle tables, which the importer writes to bullets.json.
    bool loadTables(const std::string &path);

    // AS: addBullet(x, y, dirx, diry, type, package, unused, statAttack, fromEnemy, fromWeaponName). Null when the
    // class is not ported.
    models::Bullet *addBullet(double x, double y, double dirx, double diry, const std::string &type,
                              const std::string &package = "", bool unused = true, double statAttack = 1,
                              bool fromEnemy = false, const std::string &fromWeaponName = "none");
    void processBullets();
    void clearBullets();
    void destroy() { clearBullets(); } // AS: BulletManager.destroy
    void addToPreblit(const std::string &type);
    void preblitSheets();
    void removeBullet(double bulletid, bool nosplice = false);

    obf_9G::obf_4R *gameObj = nullptr;
    double counter = 0;
    std::vector<models::Bullet *> bullets; // null where removeBullet(id, true) left a hole (the original's 0)
    double bossScythes = 0;
    std::vector<std::string> preblitSheetNames;
    std::vector<std::string> bulletTypes;
    std::vector<const engine::Sheet *> bulletSheets;

private:
    std::map<double, std::unique_ptr<models::Bullet>> byID_;
    std::vector<std::unique_ptr<models::Bullet>> graveyard_; // removed bullets wait a frame (see EnemyManager)
    std::map<std::string, models::CycleTable> tables_;
};

} // namespace papaGame::managers
