#pragma once
// AS: papaGame.models.enemies.Enemy16 — the dill wheel: rolls along floors, walls and ceilings, round every corner,
// easing over the edges. Any stun hurts it.
#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy16 : public Enemy {
public:
    using Enemy::Enemy;

    void init(double xtile, double ytile, double id, double whichScreen, double whichTileset, double screenObjID,
              const std::vector<as3::Value> &extraParams) override;
    void defineVars() override;
    void setupParams(const std::vector<as3::Value> &params) override;
    void reactToHurtingPlayer() override {} // AS: Enemy16.reactToHurtingPlayer (empty)
    void stunEnemy() override;
    const AnimationCycle &grabAnimationCycle() override;
    bool checkCloudTile(double tx, double ty, bool below = false) const;
    bool checkForCollision(double tx, double ty, bool below = false) const;
    bool checkForSlope(double tx, double ty) const;
    bool checkForCeilingSlope(double tx, double ty) const;
    void updateObject() override;
    bool moveChar(double dirx, double diry, double jumpDir, bool bounceAtWalls = false, bool checkEdges = false,
                  bool collide = true, double fixedSpeed = INT_MAX) override;
    void findStartupSurface();
    void checkForCeilSlopes(double dirx, double diry);
    double adjustSpeedForSlopes(double speed) const override;
    bool checkOnScreen() override;
    void setupTransition(double targetX, double targetY);
    void killMe(double dir = 1) override;

    double originalDirX = 1;
    double wallDir = 0;
    double onCeilingSlope = 0;
    bool onCeilingSlopeWallLeft = false;
    bool onCeilingSlopeWallRight = false;
    bool isTransitioning = false;
    double transitionTargetX = 0;
    double transitionTargetY = 0;
    double transitionStartX = 0;
    double transitionStartY = 0;
    double startTransition = 0;
    double transTimer = 0;
    double transitionDuration = 0;
};

} // namespace papaGame::models::enemies
