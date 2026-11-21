#pragma once
// AS: papaGame.models.Character — position, tile collision and movement shared by the player and enemies.
// Ported so far: getMyCorners, cloud/ladder/edge checks, fallChar, jumpChar, moveChar.
// Also checkForSlopes / checkForCeilingSlopes, the sprite animation (updateSprite, spriteChangeFrame), health
// and sprite collisions. SKELETON: crawl gaps and doors.
#include <climits>
#include <map>
#include <string>
#include <vector>

namespace engine { struct JsonValue; }
namespace obf_9G { class obf_4R; }

namespace papaGame::models {

class GameObject;

// One `...CycleFrames` array: [name, type, blitStyle, randomize, loopFrame, frames] (type 2), or
// [name, type, blitStyle, randomize, loopFrame, firstFrame, lastFrame] (type 1).
struct AnimationCycle {
    std::string name;
    double type = 2;
    double blitStyle = 1;
    double randomize = 0;
    double loopFrame = 0; // -1: play once and call endAnimationCycle
    std::vector<double> frames;
    double firstFrame = 0, lastFrame = 0;

    // cycle[5][index]; reading past the end gives undefined, which is NaN as a Number.
    double frameAt(double index) const;
};

// Port: the array literals a class assigns in code, [name, 1, blitStyle, randomize, loopFrame, first, last] and
// [name, 2, blitStyle, randomize, loopFrame, [frames]].
inline AnimationCycle cycleType1(std::string name, double blitStyle, double randomize, double loopFrame, double first,
                                 double last)
{
    return {std::move(name), 1, blitStyle, randomize, loopFrame, {}, first, last};
}
inline AnimationCycle cycleType2(std::string name, double blitStyle, double randomize, double loopFrame,
                                 std::vector<double> frames)
{
    return {std::move(name), 2, blitStyle, randomize, loopFrame, std::move(frames), 0, 0};
}

// Port: a class's `...CycleFrames` constants by field name, as the importer writes them.
using CycleTable = std::map<std::string, AnimationCycle>;
void parseCycleTable(const engine::JsonValue &fields, CycleTable &out);

// A flash.geom.Rectangle.
struct Rect {
    double x = 0, y = 0, width = 0, height = 0;
    bool intersects(const Rect &o) const; // AS: Rectangle.intersects
};
// Port: a [x, y, w, h] constant from the importer (zero if missing).
Rect rectField(const engine::JsonValue &fields, const char *name);

class Character {
public:
    explicit Character(obf_9G::obf_4R *gameObj_) : gameObj(gameObj_) {}
    virtual ~Character() = default;

    virtual void defineVars() {}
    // Port: remember the position before a logic step (interpolation).
    void savePrevious() { prevX = x; prevY = y; }

    // AS: Character.updateObject / destroy / killMe (empty in Character).
    virtual void updateObject() {}
    virtual void destroy() {}
    virtual void killMe(double /*dir*/ = 1) {}
    void adjustHealth(double amount);
    // Animation: grabAnimationCycle picks the cycle; updateSprite advances it.
    virtual const AnimationCycle &grabAnimationCycle();
    virtual void randomizeAnimationCycle() {}
    virtual void endAnimationCycle() {}
    // The original also passes the old frame numbers and the cycle type, which no override reads.
    virtual void adjustFrameSync(const std::string &newName, const std::string &oldName, double firstFrame,
                                 const AnimationCycle &cycle);
    virtual void updateSprite();
    void spriteChangeFrame(double step, const AnimationCycle &cycle, double type);
    // Port: `this[field]` for a cycle array; a missing one falls back to standing.
    const AnimationCycle &cycleFrames(const std::string &field) const;
    bool checkSpriteCollision(const Rect &r, const Rect *rect = nullptr) const;
    Rect getCurrentCollRect(const Rect *rect = nullptr) const;
    void setSheetIndex(double index) { sheetIndex = index; }
    double getProximityVolume() const;
    double getProximityPan() const;
    bool isReallyOnScreen() const;
    virtual bool checkIfOnCloud(); // overridden by Basketball
    double checkIfOnSlope(double px, double py, bool ignoreJump = false) const;
    bool checkIfLadderBelow();
    bool checkIfHalfOnCloud(double dir);
    bool checkMovingTiles(double y);
    virtual void checkIfAtEdge(double speed, double dir);
    virtual bool moveChar(double dirx, double diry, double jumpDir, bool bounceAtWalls = false, bool checkEdges = false,
                          bool collide = true, double fixedSpeed = INT_MAX); // overridden by Enemy16
    virtual void fallChar();
    virtual void jumpChar(bool collide = true);
    void getMyCorners(double px, double py);
    void checkForCeilingSlopes(double dir = -1);
    void checkForSlopes(double dirx, double diry);
    virtual bool checkForCrawlGap(double /*dir*/) { return false; }
    virtual bool isInsideCrawlGap() { return false; } // AS: Character.isInsideCrawlGap
    bool checkDropThruCloud() const { return false; } // AS: Character.checkDropThruCloud (always false)

    // PlayerChar hooks used by moveChar (overridden by the player).
    virtual void checkGoingThroughDoor(double /*dx*/, double /*dy*/) {}
    virtual bool isPlayer() const { return false; }
    virtual bool isFinishingLevelPlayer() const { return false; }
    virtual void fellOffLevel(double /*maxY*/) {}

    obf_9G::obf_4R *gameObj = nullptr;
    double id = 0;
    double type = 0;
    double damageAmount = 1;
    std::string sheetname;
    bool sheetIsBitmap = true;
    double sheetIndex = -1;
    Rect collRect{0, 0, 32, 32};
    bool facingWalking = false;
    double spriteCenterX = 64, spriteCenterY = 40, spriteTargetX = 75, spriteTargetY = 75;
    double sheetWidth = 18, sheetHeight = 18, flipBlitOffset = 0;
    bool isHit = false, isStunned = false, isTurning = false;
    double stunStartTime = 0, stunDuration = 90, hitStartTime = 0, hitDuration = 700;
    double maxHealth = 5, currentHealth = 5;
    double animationFrame = 0, cycleFrame = 0, animationBlitStyle = 1;
    std::string cycleName;
    const CycleTable *cycleTable = nullptr; // port: the class's cycle constants
    double prevX = 0, prevY = 0;            // port: interpolation
    double x = 0, y = 0, xtile = 0, ytile = 0;
    double height = 15, width = 15, playerheight = 60;
    double spriteheight = 80, spritewidth = 128;
    double heightmultiplier = 1, widthmultiplier = 1;
    double speed = 7, walkspeed = 7, jumpspeed = 0, jumpstart = -26;
    double gravity = 2, normalgravity = 2, jumpgravity = 2, glidegravity = 1;
    double baseSlideSpeed = 0, slideSpeed = 0;
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
    GameObject *whichMovingTile = nullptr; // the object it stands on
    double onSlope = 0;  // slope type under the character (1-10), 0 = none
    double slopeDir = 0;
    bool onCeiling = false, onlyCheckSolidSlopes = false, onGround = false;
    bool onWall = false, canClimbUp = false, bounceX = false, bounceY = false;
    bool shouldCheckCeilingSlopes = false;
    double bounceDecayX = 1, bounceDecayY = 1, numBounces = 0;

    // getMyCorners results
    double downY = 0, upY = 0, standupY = 0, standupmidY = 0, leftX = 0, rightX = 0;
    double climbDir = 1;
    double midY = 0, downmidY = 0, upmidY = 0;
    bool HitUpLeft = false, HitDownLeft = false, HitUpRight = false, HitDownRight = false;
    bool HitMidLeft = false, HitMidRight = false, HitUpMidLeft = false, HitUpMidRight = false;
    bool HitDownMidLeft = false, HitDownMidRight = false, HitStandUpLeft = false, HitStandUpRight = false;
    double SlopeDownLeft = 0, SlopeDownRight = 0, SlopeLeft = 0, SlopeRight = 0;
};

} // namespace papaGame::models
