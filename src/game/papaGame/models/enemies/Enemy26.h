#pragma once
// AS: papaGame.models.enemies.Enemy26 — the Cheddar Mack: a cheese hanging on a rope from the ceiling above it,
// always swinging (pushed by hits and stomps); beaten, it drops off its rope.
#include <vector>

#include "game/papaGame/models/Enemy.h"

namespace papaGame::models::enemies {

class Enemy26 : public Enemy {
public:
    using Enemy::Enemy;

    void init(double xtile, double ytile, double id, double whichScreen, double whichTileset, double screenObjID,
              const std::vector<as3::Value> &extraParams) override;
    void defineVars() override;
    void setupParams(const std::vector<as3::Value> & /*params*/) override {} // AS: Enemy26.setupParams (empty)
    bool getHit(double damage = 1, double dir = 1, bool unused = false, const std::string &weapon = "none") override;
    void stunEnemy() override;
    void reactToHurtingPlayer() override {} // AS: Enemy26.reactToHurtingPlayer (empty)
    const AnimationCycle &grabAnimationCycle() override;
    void updateObject() override;
    void startupEnemy();
    double convertToRadians(double degrees) const;
    double convertToDegrees(double radians) const;
    void setupSwing(double dir = 1, bool fromCurrentAngle = false);
    void swingChar();
    void checkBeforeBlitting() override;
    bool checkOnScreen() override;
    void redrawLine();
    void updateAnimationFrame();
    void killMe(double dir = 1) override;

    // The original's swingCycleFrames array, whose one frame updateAnimationFrame rewrites from the rope's angle.
    AnimationCycle swingCycleFrames = cycleType2("swing", 1, 0, 0, {16});
    bool hasAnchorTile = false; // port: anchorTile is not null
    double anchorTileX = 0, anchorTileY = 0;
    double lastPlayedSound = 0;
    double maxAngle = 60;
    double anchorX = as3::NaN;
    double anchorY = as3::NaN;
    double initialAngle = as3::NaN;
    double currentAngle = as3::NaN;
    double workingAngle = as3::NaN;
    double targetAngle = as3::NaN;
    double currentVelocity = as3::NaN;
    double lineLength = as3::NaN;
    double timeElapsed = as3::NaN;
    double changeSlackDir = 0;
    double pumpAdjustor = 0.98;
    double pumpDropDelay = 2;
    double pumpDropTimer = 0;
    double pumpAmount = 0.015;
    double gravityDecay = 0.98;
    bool collidedWhileSwinging = false;
    bool swingInWall = false;
    double ropeOffsetX = 0;
    double ropeOffsetY = 0;
    double swingSpeed = 6;
    bool isSwinging = false;
    bool isConnected = true;
    double slackSpeed = 6;
    // The hit box's corner for each animation frame.
    std::vector<double> collX{-30, -29, -28, -27, -26, -25, -24, -23, -22, -21, -20, -19, -18, -17, -16, -15, -14, -13,
                              -12, -11, -10, -9,  -8,  -7,  -6,  -5,  -4,  -3,  -2,  -1,  0,   2,   3,   -14, -14, -14};
    std::vector<double> collY{2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 9,
                              9, 8, 8, 7, 7, 6, 5, 5, 4, 4, 3, 3, 2, 1, 1, 10, 10, 10};
};

} // namespace papaGame::models::enemies
