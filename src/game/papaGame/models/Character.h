#pragma once
// AS: papaGame.models.Character — position, tile collision and movement shared by the player and enemies.
// Ported so far: getMyCorners, cloud/ladder/edge checks, fallChar, jumpChar, moveChar.
// Also checkForSlopes / checkForCeilingSlopes. SKELETON: moving tiles, crawl gaps and doors.
#include <climits>

namespace obf_9G { class obf_4R; }

namespace papaGame::models {

class Character {
public:
    explicit Character(obf_9G::obf_4R *gameObj_) : gameObj(gameObj_) {}
    virtual ~Character() = default;

    virtual void defineVars() {}
    bool checkIfOnCloud();
    bool checkIfLadderBelow();
    bool checkIfHalfOnCloud(double dir);
    bool checkMovingTiles(double y);
    void checkIfAtEdge(double speed, double dir);
    bool moveChar(double dirx, double diry, double jumpDir, bool bounceAtWalls = false, bool checkEdges = false,
                  bool collide = true, double fixedSpeed = INT_MAX);
    void fallChar();
    void jumpChar(bool collide = true);
    void getMyCorners(double px, double py);
    void checkForCeilingSlopes(double dir = -1);
    void checkForSlopes(double dirx, double diry);
    virtual bool checkForCrawlGap(double /*dir*/) { return false; }
    bool checkDropThruCloud() const { return false; } // AS: Character.checkDropThruCloud (always false)

    // PlayerChar hooks used by moveChar (overridden by the player).
    virtual void checkGoingThroughDoor(double /*dx*/, double /*dy*/) {}
    virtual bool isPlayer() const { return false; }
    virtual bool isFinishingLevelPlayer() const { return false; }
    virtual void fellOffLevel() {}

    obf_9G::obf_4R *gameObj = nullptr;
    double x = 0, y = 0, xtile = 0, ytile = 0;
    double height = 15, width = 15, playerheight = 60;
    double spriteheight = 80, spritewidth = 128;
    double heightmultiplier = 1, widthmultiplier = 1;
    double speed = 7, walkspeed = 7, jumpspeed = 0, jumpstart = -26;
    double gravity = 2, normalgravity = 2, jumpgravity = 2;
    double maxjumpvelocity = INT_MAX;
    double shortjump = -10;
    double slopedIntoCeiling = 0;
    double jumpedFromSlope = 0;
    double lastx = 0, lasty = 0;
    double lastOnSlope = 0;
    double facingDir = -1, walkingDir = 1, dirx = 0, diry = 0;
    double jumpedFromX = 0, jumpedFromY = 0;
    bool jump = false, triggerjump = false, triggerjumpx = false;
    bool isWalking = false, isSliding = false, isCrawling = false, isClimbingLadder = false;
    bool isFlying = false, isGliding = false, isWallSliding = false, isInQuicksand = false;
    bool isCarryingObject = false, isDead = false, duck = false, canCrawl = false;
    bool onMovingTile = false, onSlopeWallLeft = false, onSlopeWallRight = false;
    double onSlope = 0;  // slope type under the character (1-10), 0 = none
    double slopeDir = 0;
    bool onCeiling = false, onlyCheckSolidSlopes = false;
    bool onWall = false, canClimbUp = false, bounceX = false, bounceY = false;
    bool shouldCheckCeilingSlopes = false;
    double bounceDecayX = 1, bounceDecayY = 1, numBounces = 0;

    // getMyCorners results
    double downY = 0, upY = 0, standupY = 0, standupmidY = 0, leftX = 0, rightX = 0;
    double midY = 0, downmidY = 0, upmidY = 0;
    bool HitUpLeft = false, HitDownLeft = false, HitUpRight = false, HitDownRight = false;
    bool HitMidLeft = false, HitMidRight = false, HitUpMidLeft = false, HitUpMidRight = false;
    bool HitDownMidLeft = false, HitDownMidRight = false, HitStandUpLeft = false, HitStandUpRight = false;
    double SlopeDownLeft = 0, SlopeDownRight = 0, SlopeLeft = 0, SlopeRight = 0;
};

} // namespace papaGame::models
