#include "game/papaGame/models/characters/CustomerChar.h"

#include <cmath>

#include "engine/json.h"
#include "game/as3.h"
#include "game/obf_9G/obf_4R.h"
#include "game/obf_AF/obf_1n.h"
#include "game/papaGame/models/effects/DizzyEffect.h"

using obf_AF::obf_1n;
using papaGame::data::CustomerData;

namespace papaGame::models::characters {

namespace {

// Speeds on slopes are scaled by the cosine of the slope angle: types 1-2 are 45 degrees, 3-6 are 22 and
// 7-10 the steep ones, whose angle the callers give.
double slopeSpeed(double slope, double speed, double steepAngle)
{
    if (slope == 1 || slope == 2) {
        return speed * std::cos(obf_1n::degreesToRads(45));
    }
    if (slope == 3 || slope == 4 || slope == 5 || slope == 6) {
        return speed * std::cos(obf_1n::degreesToRads(22));
    }
    if (slope == 7 || slope == 8 || slope == 9 || slope == 10) {
        return speed * std::cos(obf_1n::degreesToRads(steepAngle));
    }
    return speed;
}

} // namespace

CustomerChar::CustomerChar(obf_9G::obf_4R *gameObj_) : Character(gameObj_)
{
    defineVars(); // AS: Character's constructor calls defineVars()
}

// AS: CustomerChar.defineVars (collision rectangles, health and the skill flags other than crawling come later)
void CustomerChar::defineVars()
{
    height = 15;
    width = 15;
    spritewidth = 74;
    spriteheight = 60;
    heightmultiplier = 5;
    widthmultiplier = 1;
    speed = 8;
    walkspeed = 8;
    jumpspeed = 0;
    jumpstart = -23;
    gravity = 2;
    normalgravity = 2;
    jumpgravity = 2;
    stunDuration = 60;
    collRect = {-15, -80, 30, 90};
    standCollRect = {-8, -63, 16, 73};
    jumpCollRect = {-8, -63, 16, 73};
    duckCollRect = {-8, -20, 16, 30};
    currentHealth = 3;
    maxHealth = 3;
    shouldCheckCeilingSlopes = true;
    if (const data::CustomerDataFile *c = playerData()) {
        hasWeaponType = c->weaponType;
        canCrawl = c->skillType == CustomerData::SKILL_CRAWL;
        if (canCrawl) {
            playerheight = 50;
        }
        jumpstart = c->jumpstart;
        baseWalkSpeed = c->walkspeed;
        walkspeed = baseWalkSpeed;
        maxSkidSpeed = c->maxskidspeed;
        skidDecayTimerInterval = c->skiddecay;
        skidDecayAmount = c->skiddecayamount;
    }
}

const data::CustomerDataFile *CustomerChar::playerData() const
{
    return gameObj->obf_3A->getCustomerData(gameObj->obf_51->selectedCharacter);
}

bool CustomerChar::loadTables(const std::string &path)
{
    size_t size = 0;
    char *raw = static_cast<char *>(SDL_LoadFile(path.c_str(), &size));
    if (!raw) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot read %s (import the game again)", path.c_str());
        return false;
    }
    engine::JsonValue tables;
    const bool ok = engine::parseJson(std::string_view(raw, size), tables);
    SDL_free(raw);
    if (!ok) {
        return false;
    }
    auto numbers = [](const engine::JsonValue &v) {
        std::vector<double> out;
        for (const auto &n : v.items) {
            out.push_back(n.number);
        }
        return out;
    };
    parseCycleTable(tables, cycles_);
    cycleTable = &cycles_;
    auto list = [&](const char *name) {
        const engine::JsonValue *v = tables.get(name);
        return v ? numbers(*v) : std::vector<double>{};
    };
    torsoOffsetsX_ = list("torsoOffsetsX");
    torsoOffsetsY_ = list("torsoOffsetsY");
    stomp_collRect = rectField(tables, "stomp_collRect");
    pound_collRect = rectField(tables, "pound_collRect");
    melee_attackRect = rectField(tables, "melee_attackRect");
    swing_attackRect = rectField(tables, "swing_attackRect");
    whip_attackRect = rectField(tables, "whip_attackRect");
    other_attackRect = rectField(tables, "other_attackRect");
    melee_frameRange = list("melee_frameRange");
    swing_frameRange = list("swing_frameRange");
    whip_frameRange = list("whip_frameRange");
    other_frameRange = list("other_frameRange");
    return cycles_.count("standCycleFrames") > 0;
}

// AS: PlayerChar.startupPlayer
void CustomerChar::startupPlayer(double tileX, double tileY, bool newLevel, bool keepFacing)
{
    const display::GameDisplay &display = *gameObj->obf_71;
    x = tileX * display.tileWidth + display.tileWidth / 2;
    y = tileY * display.tileWidth + display.tileWidth / 2;
    xtile = tileX;
    ytile = tileY;
    if (!keepFacing) {
        facingDir = 1;
        walkingDir = 1;
    }
    lastSafeX = tileX;
    lastSafeY = tileY;
    isWalking = false;
    isDead = false;
    isHit = false;
    fellOffTrain = false;
    stunStarIDs.clear();
    if (!keepFacing) {
        statAttack = 1;
    }
    canPressLeft = true;
    canPressRight = true;
    gameObj->obf_8o->stopHurtControls = false;
    if (!keepFacing) {
        // A fresh start begins stunned: the flicker is the start-up invincibility.
        isStunned = true;
        stunStartTime = 0;
    }
    if (isInvincible) {
        isInvincible = false;
        if (invincibleFXID > -1) {
            gameObj->obf_V->removeEffect(invincibleFXID);
            invincibleFXID = -1;
        }
    }
    if (newLevel) {
        isFinishingLevel = false;
        hitByBlast = false;
        gameObj->obf_8o->stopActionControls = false;
    }
    if (!keepFacing) {
        currentHealth = maxHealth;
        if (gameObj->obf_6Z) {
            gameObj->obf_6Z->updatePlayerHealth(currentHealth);
        }
    }
    if (newLevel) {
        gameObj->obf_V->addEffect(x, y - 64, "EndPortalEffect", "startup");
    }
    // SKELETON: the skill tutorials (§_-6n§).
    fallChar();
    prevX = x;
    prevY = y;
}

// AS: PlayerChar.canFudgeJump — a few frames after walking off a ledge a jump still counts.
bool CustomerChar::canFudgeJump() const
{
    return gameObj->obf_8o->gameplayTimer < lastSafeTime + lastSafeTimeBuffer && !duck && !isCrawling && !isDucking &&
           (!jump || jumpspeed >= 0);
}

// AS: PlayerChar.startDucking
void CustomerChar::startDucking()
{
    // SKELETON: on a slope the original starts sliding (startSliding) instead.
    duck = true;
    isDucking = true;
    isTurning = false;
}

// AS: PlayerChar.checkIfOnLadder — a ladder tile here; going down it must not start on solid ground.
bool CustomerChar::checkIfOnLadder(double dir)
{
    const managers::MapManager &map = *gameObj->obf_4i;
    if (map.getTileProperty(xtile, ytile, "ladder") > 0) {
        if (!isClimbingLadder && dir == 1 &&
            (map.getTileProperty(xtile, ytile + 1, "collision") > 0 ||
             map.getTileProperty(xtile, ytile + 1, "thrublock") > 0)) {
            return false;
        }
        // SKELETON: false while cheesed.
        return true;
    }
    return false;
}

// AS: PlayerChar.checkIfOnDoor — standing in a doorway arms it; walking out the door's side enters it.
void CustomerChar::checkIfOnDoor()
{
    const managers::MapManager &map = *gameObj->obf_4i;
    const std::vector<double> door = map.getTileDoor(xtile, ytile);
    if (!door.empty() && !isDead) {
        waitToEnterDoor = true;
        doorDirX = door[4];
        doorDirY = door[5];
        doorXtile = xtile;
        doorYtile = ytile;
    } else if (map.isWithinBounds(xtile, ytile)) {
        waitToEnterDoor = false;
    }
}

// AS: PlayerChar.checkGoingThroughDoor — called by moveChar when the player hits a level edge.
void CustomerChar::checkGoingThroughDoor(double dx, double dy)
{
    if (waitToEnterDoor && !gameObj->obf_71->isTransitioningIn) {
        if (dx == doorDirX && dy == doorDirY) {
            enterDoor(doorXtile, doorYtile);
        }
    }
}

// AS: PlayerChar.enterDoor
void CustomerChar::enterDoor(double tileX, double tileY)
{
    const std::vector<double> door = gameObj->obf_4i->getTileDoor(tileX, tileY);
    if (door.empty()) {
        return;
    }
    const double dirY = door[5];
    cancelAttack();
    if (dirY == -1) {
        jump = true; // going up: jump into the room above
        jumpspeed = jumpstart;
    } else {
        jump = false;
    }
    gameObj->obf_8o->travelThroughDoor(door[0] - 1, door[1], door[2], door[3], door[4], dirY);
}

// Character.moveChar, below the bottom edge of the level (the player part).
void CustomerChar::fellOffLevel(double maxY)
{
    if (isDead) {
        restartPlayer();
    } else {
        y = maxY + heightmultiplier * height - 3;
        if (!isRidingElevator) { // (no trains in PL2: DataManager.isOnTrain is always false)
            hurtPlayer(0, INT_MAX, true);
        }
        checkGoingThroughDoor(0, 1);
    }
}

// AS: CustomerChar.getTorsoOffsetX
double CustomerChar::getTorsoOffsetX() const
{
    if (isAttacking) {
        return footAnimationFrame >= 0 && footAnimationFrame < static_cast<double>(torsoOffsetsX_.size())
                   ? torsoOffsetsX_[static_cast<size_t>(footAnimationFrame)]
                   : as3::NaN;
    }
    return 0;
}

// AS: CustomerChar.getTorsoOffsetY
double CustomerChar::getTorsoOffsetY() const
{
    if (isAttacking) {
        return footAnimationFrame >= 0 && footAnimationFrame < static_cast<double>(torsoOffsetsY_.size())
                   ? torsoOffsetsY_[static_cast<size_t>(footAnimationFrame)]
                   : as3::NaN;
    }
    return 0;
}

// AS: CustomerChar.grabAnimationCycle
const AnimationCycle &CustomerChar::grabAnimationCycle()
{
    const data::CustomerDataFile *data = playerData();
    const std::string &weaponType = data->weaponType;
    const bool crawlSkill = data->skillType == CustomerData::SKILL_CRAWL;
    if (isGone) {
        return cycleFrames("goneCycleFrames");
    }
    if (isDead) {
        if (isCrawling) {
            return cycleFrames("crawldeadCycleFrames");
        }
        if (jump) {
            return cycleFrames("jump_hurtCycleFrames"); // rising or falling alike
        }
        return cycleFrames("deadCycleFrames");
    }
    if (isHit) {
        if (isCrawling) {
            return cycleFrames("hurtcrawlCycleFrames");
        }
        if (jump) {
            return cycleFrames("jump_hurtCycleFrames");
        }
        return cycleFrames("skidCycleFrames");
    }
    if (isRidingObject || isGrabbing) {
        return cycleFrames("grabCycleFrames");
    }
    if (isClimbingLadder) {
        return cycleFrames("climbCycleFrames");
    }
    if (isAttacking) {
        if (weaponType == CustomerData::WEAPON_MELEE) {
            return cycleFrames("attack" + as3::String(attackVariation) + "_meleeCycleFrames");
        }
        if (weaponType == CustomerData::WEAPON_LONGGUN) {
            return cycleFrames("shoot_longgunCycleFrames");
        }
        if (weaponType == CustomerData::WEAPON_PISTOL) {
            return cycleFrames("shoot_pistolCycleFrames");
        }
        if (weaponType == CustomerData::WEAPON_BAZOOKA) {
            return cycleFrames("shoot_bazookaCycleFrames");
        }
        if (attackVariation == 2) {
            return cycleFrames("attack2_swing1CycleFrames");
        }
        return cycleFrames("attack1_swing1CycleFrames");
    }
    if (isFinishPushing) {
        isTurning = false;
        return cycleFrames("finishpushCycleFrames");
    }
    if (isSliding) {
        return cycleFrames(slideCycleName(data->customerName) + "CycleFrames");
    }
    if (isGroundPounding) {
        return cycleFrames("poundCycleFrames");
    }
    if (isWallSliding) {
        return cycleFrames("wallslideCycleFrames");
    }
    if (isPushingObject) {
        return cycleFrames("pushCycleFrames");
    }
    if (isTurning && !jump) {
        if (duck && crawlSkill) {
            return cycleFrames("turncrawlCycleFrames");
        }
        return cycleFrames("turnaroundCycleFrames");
    }
    if (isGliding) {
        return cycleFrames("glideCycleFrames");
    }
    if (jump) {
        isTurning = false;
        isSkidding = false;
        if (isDoubleJumping) {
            return cycleFrames(jumpspeed < 0 ? "doublejumpCycleFrames" : "doublefallCycleFrames");
        }
        if (isFinishingLevel) {
            return cycleFrames(jumpspeed < 0 ? "jumphappyCycleFrames" : "fallhappyCycleFrames");
        }
        return cycleFrames(jumpspeed < 0 ? "jumpCycleFrames" : "fallCycleFrames");
    }
    if (isWalking && !isSkidding) {
        if (!isPushingObject && duck && crawlSkill) {
            return cycleFrames("crawlCycleFrames");
        }
        return cycleFrames("runCycleFrames");
    }
    if (duck) {
        return cycleFrames("duckCycleFrames");
    }
    if (isDucking) {
        return cycleFrames("unduckCycleFrames");
    }
    if (isSkidding) {
        return cycleFrames("skidCycleFrames");
    }
    return cycleFrames("standCycleFrames");
}

// AS: CustomerChar.grabFootAnimationCycle
const AnimationCycle &CustomerChar::grabFootAnimationCycle()
{
    const data::CustomerDataFile *data = playerData();
    const std::string &weaponType = data->weaponType;
    const std::string &name = data->customerName;
    const bool crawlSkill = data->skillType == CustomerData::SKILL_CRAWL;
    if (isGone) {
        return cycleFrames("foot_goneCycleFrames");
    }
    if (isDead) {
        if (isCrawling) {
            return cycleFrames("foot_crawldeadCycleFrames");
        }
        if (jump) {
            return cycleFrames(jumpspeed < 0 ? "foot_jump_hurtCycleFrames" : "foot_fall_hurtCycleFrames");
        }
        return cycleFrames("foot_deadCycleFrames");
    }
    if (isHit) {
        if (isCrawling) {
            return cycleFrames("foot_hurtcrawlCycleFrames");
        }
        if (jump) {
            return cycleFrames(jumpspeed < 0 ? "foot_jump_hurtCycleFrames" : "foot_fall_hurtCycleFrames");
        }
        return cycleFrames("foot_skidCycleFrames");
    }
    if (isRidingObject || isGrabbing) {
        return cycleFrames("foot_grabCycleFrames");
    }
    if (isClimbingLadder) {
        return cycleFrames("foot_climbCycleFrames");
    }
    if (isFinishPushing) {
        return cycleFrames("foot_finishpushCycleFrames");
    }
    if (isSliding) {
        return cycleFrames("foot_" + slideCycleName(name) + "CycleFrames");
    }
    if (isWallSliding) {
        return cycleFrames("foot_wallslideCycleFrames");
    }
    if (isPushingObject) {
        return cycleFrames("foot_pushCycleFrames");
    }
    if (isGroundPounding) {
        return cycleFrames("foot_poundCycleFrames");
    }
    const std::string variation = as3::String(attackVariation);
    if (isAttacking && name == "Ninjoy" && (attackVariation == 4 || attackVariation == 2)) {
        return cycleFrames("foot_attack" + variation + "_meleeCycleFrames");
    }
    if (isAttacking && name == "Clover" && (attackVariation == 4 || attackVariation == 3)) {
        return cycleFrames("foot_attack" + variation + "_meleeCycleFrames");
    }
    if (isTurning) {
        if (duck && crawlSkill) {
            return cycleFrames("foot_turncrawlCycleFrames");
        }
        return cycleFrames("foot_turnaroundCycleFrames");
    }
    if (isGliding) {
        return cycleFrames("foot_glideCycleFrames");
    }
    if (jump) {
        if (isDoubleJumping) {
            return cycleFrames(jumpspeed < 0 ? "foot_doublejumpCycleFrames" : "foot_doublefallCycleFrames");
        }
        if (isFinishingLevel) {
            return cycleFrames(jumpspeed < 0 ? "foot_jumphappyCycleFrames" : "foot_fallhappyCycleFrames");
        }
        return cycleFrames(jumpspeed < 0 ? "foot_jumpCycleFrames" : "foot_fallCycleFrames");
    }
    if (isWalking && !isSkidding) {
        if (duck && crawlSkill) {
            return cycleFrames("foot_crawlCycleFrames");
        }
        return cycleFrames("foot_runCycleFrames");
    }
    if (duck) {
        return cycleFrames("foot_duckCycleFrames");
    }
    if (isDucking) {
        return cycleFrames("foot_unduckCycleFrames");
    }
    if (isSkidding) {
        return cycleFrames("foot_skidCycleFrames");
    }
    if (isAttacking) {
        if (weaponType == CustomerData::WEAPON_MELEE) {
            return cycleFrames("foot_attack" + variation + "_meleeCycleFrames");
        }
        if (weaponType == CustomerData::WEAPON_LONGGUN) {
            return cycleFrames("foot_shoot_longgunCycleFrames");
        }
        if (weaponType == CustomerData::WEAPON_PISTOL) {
            return cycleFrames("foot_shoot_pistolCycleFrames");
        }
        if (weaponType == CustomerData::WEAPON_BAZOOKA) {
            return cycleFrames("foot_shoot_bazookaCycleFrames");
        }
        if (attackVariation == 2) {
            return cycleFrames("foot_attack2_swing1CycleFrames");
        }
        return cycleFrames("foot_attack1_swing1CycleFrames");
    }
    return cycleFrames("foot_standCycleFrames");
}

// The sliding branch of grabAnimationCycle / grabFootAnimationCycle (the same table in both): Scooter's
// board tilts with the slope under it; everyone else uses "slide".
std::string CustomerChar::slideCycleName(const std::string &customerName) const
{
    if (customerName != "Scooter") {
        return "slide";
    }
    const double s = onSlope;
    const double f = facingDir;
    if (s == 1 && f == 1) return "slide45up";
    if ((s == 3 || s == 4) && f == 1) return "slide22up";
    if ((s == 7 || s == 8) && f == 1) return "slide67up";
    if (s == 1 && f == -1) return "slide45down";
    if ((s == 3 || s == 4) && f == -1) return "slide22down";
    if ((s == 7 || s == 8) && f == -1) return "slide67down";
    if (s == 2 && f == -1) return "slide45up";
    if ((s == 5 || s == 6) && f == -1) return "slide22up";
    if ((s == 9 || s == 10) && f == -1) return "slide67up";
    if (s == 2 && f == 1) return "slide45down";
    if ((s == 5 || s == 6) && f == 1) return "slide22down";
    if ((s == 9 || s == 10) && f == 1) return "slide67down";
    return "slide";
}

// AS: CustomerChar.updateSprite — the feet first, then the torso.
void CustomerChar::updateSprite()
{
    double step = 1;
    const AnimationCycle &foot = grabFootAnimationCycle();
    if (foot.name == "foot_climb") {
        step = climbDir;
    }
    double firstFrame = foot.type == 1 ? foot.firstFrame : foot.frameAt(0);
    if (foot.name != footCycleName) {
        adjustFootFrameSync(foot.name, footCycleName, firstFrame, foot);
    } else {
        adjustFootFrameSync(foot.name, footCycleName, firstFrame, foot);
        footSpriteChangeFrame(step, foot, foot.type);
    }
    const AnimationCycle &torso = grabAnimationCycle();
    firstFrame = torso.type == 1 ? torso.firstFrame : torso.frameAt(0);
    if (torso.name != cycleName) {
        adjustFrameSync(torso.name, cycleName, firstFrame, torso);
    } else {
        adjustFrameSync(torso.name, cycleName, firstFrame, torso);
        spriteChangeFrame(step, torso, torso.type);
    }
    animationBlitStyle = torso.blitStyle;
}

// AS: CustomerChar.footSpriteChangeFrame
void CustomerChar::footSpriteChangeFrame(double step, const AnimationCycle &cycle, double cycleType)
{
    if (cycleType == 1) {
        footAnimationFrame += step;
        const double loopFrame = cycle.loopFrame;
        const double lastFrame = cycle.lastFrame;
        if (footAnimationFrame > lastFrame) {
            footAnimationFrame = loopFrame == -1 ? lastFrame : loopFrame;
        }
        if (footAnimationFrame < loopFrame && facingDir != walkingDir) {
            footAnimationFrame = lastFrame;
        }
    } else if (cycleType == 2) {
        const double loopFrame = cycle.loopFrame;
        const double last = static_cast<double>(cycle.frames.size()) - 1;
        footCycleFrame += step;
        if (footCycleFrame < 0) {
            footCycleFrame = last;
        } else if (footCycleFrame > last) {
            footCycleFrame = loopFrame == -1 ? last : loopFrame;
        }
        footAnimationFrame = cycle.frameAt(footCycleFrame);
    }
}

// AS: CustomerChar.adjustFootFrameSync (the original also receives the old frame numbers and the cycle type,
// which it never reads).
void CustomerChar::adjustFootFrameSync(const std::string &newName, const std::string &oldName, double firstFrame,
                                       const AnimationCycle &cycle)
{
    const double last = static_cast<double>(cycle.frames.size()) - 1;
    if (newName == "foot_duck" && (oldName == "foot_crawl" || oldName == "foot_turncrawl" || oldName == "foot_hurtcrawl")) {
        footCycleFrame = last;
        footAnimationFrame = cycle.frameAt(footCycleFrame);
        footCycleName = newName;
    } else if (newName == "foot_crawl" &&
               (oldName == "foot_duck" || oldName == "foot_turncrawl" || oldName == "foot_hurtcrawl")) {
        footCycleFrame = cycle.loopFrame;
        footAnimationFrame = cycle.frameAt(footCycleFrame);
        footCycleName = newName;
    } else if ((oldName == "foot_doublejump" || oldName == "foot_doublefall") && newName == "foot_attack1_swing1" &&
               playerData()->customerClipName == "Cooper") {
        footCycleFrame = 4;
        footAnimationFrame = cycle.frameAt(footCycleFrame);
        footCycleName = newName;
    } else if (newName.starts_with("foot_slide") && (oldName.starts_with("foot_slide") || oldName.starts_with("foot_double")) &&
               newName != oldName) {
        // Scooter's board tilts one step at a time between the slide poses.
        static const std::vector<std::string> names = {"foot_slide67down", "foot_slide45down", "foot_slide22down",
                                                       "foot_slide", "foot_slide22up", "foot_slide45up",
                                                       "foot_slide67up"};
        static const double frames[] = {113, 112, 111, 68, 108, 109, 110};
        auto indexOf = [&](const std::string &n) {
            for (size_t i = 0; i < names.size(); ++i) {
                if (names[i] == n) {
                    return static_cast<int>(i);
                }
            }
            return -1;
        };
        const int to = indexOf(newName);
        int from = indexOf(oldName);
        if (oldName.starts_with("foot_double")) {
            from = indexOf("slide"); // QUIRK: looks up "slide", not "foot_slide", so this is -1
        }
        if (from < to && to - from > 1) {
            footCycleFrame = cycle.loopFrame;
            footAnimationFrame = frames[from + 1];
            footCycleName = names[static_cast<size_t>(from + 1)];
        } else if (from > to && from - to > 1) {
            footCycleFrame = cycle.loopFrame;
            footAnimationFrame = frames[from - 1];
            footCycleName = names[static_cast<size_t>(from - 1)];
        } else {
            footCycleFrame = cycle.loopFrame;
            footAnimationFrame = cycle.frameAt(cycleFrame); // QUIRK: indexed by the torso's cycleFrame
            footCycleName = newName;
        }
    } else if (newName == "foot_push" && oldName == "foot_finishpush") {
        footCycleFrame = -1;
        for (size_t i = 0; i < cycle.frames.size(); ++i) {
            if (cycle.frames[i] == footAnimationFrame) {
                footCycleFrame = static_cast<double>(i);
                break;
            }
        }
        footCycleName = newName;
    } else if (newName != oldName) {
        footAnimationFrame = firstFrame;
        footCycleFrame = 0;
        footCycleName = newName;
    }
}

// AS: CustomerChar.adjustFrameSync (the original also receives the old frame numbers and the cycle type, which
// it never reads).
void CustomerChar::adjustFrameSync(const std::string &newName, const std::string &oldName, double firstFrame,
                                   const AnimationCycle &cycle)
{
    if (newName == oldName) {
        return;
    }
    const double last = static_cast<double>(cycle.frames.size()) - 1;
    if (newName == "duck" && (oldName == "crawl" || oldName == "turncrawl" || oldName == "hurtcrawl")) {
        cycleFrame = last;
        animationFrame = cycle.frameAt(cycleFrame);
        cycleName = newName;
    } else if (newName == "crawl" && (oldName == "duck" || oldName == "turncrawl" || oldName == "hurtcrawl")) {
        cycleFrame = cycle.loopFrame;
        animationFrame = cycle.frameAt(cycleFrame);
        cycleName = newName;
    } else if (newName.find("shoot_") != std::string::npos && oldName.find("shoot_") != std::string::npos) {
        cycleName = newName;
    } else if ((oldName == "doublejump" || oldName == "doublefall") && newName == "attack1_swing1" &&
               playerData()->customerClipName == "Connor") {
        cycleFrame = 4;
        animationFrame = cycle.frameAt(cycleFrame);
        cycleName = newName;
        bulletTimer += 4;
    } else if (newName.starts_with("slide") && (oldName.starts_with("slide") || oldName.starts_with("double"))) {
        static const std::vector<std::string> names = {"slide67down", "slide45down", "slide22down", "slide",
                                                       "slide22up",   "slide45up",   "slide67up"};
        static const double frames[] = {113, 112, 111, 68, 108, 109, 110};
        auto indexOf = [&](const std::string &n) {
            for (size_t i = 0; i < names.size(); ++i) {
                if (names[i] == n) {
                    return static_cast<int>(i);
                }
            }
            return -1;
        };
        const int to = indexOf(newName);
        int from = indexOf(oldName);
        if (oldName.starts_with("double")) {
            from = indexOf("slide");
        }
        if (from < to && to - from > 1) {
            cycleFrame = cycle.loopFrame;
            animationFrame = frames[from + 1];
            cycleName = names[static_cast<size_t>(from + 1)];
        } else if (from > to && from - to > 1) {
            cycleFrame = cycle.loopFrame;
            animationFrame = frames[from - 1];
            cycleName = names[static_cast<size_t>(from - 1)];
        } else {
            cycleFrame = cycle.loopFrame;
            animationFrame = cycle.frameAt(cycleFrame);
            cycleName = newName;
        }
    } else if (newName == "push" && oldName == "finishpush") {
        cycleFrame = -1;
        for (size_t i = 0; i < cycle.frames.size(); ++i) {
            if (cycle.frames[i] == animationFrame) {
                cycleFrame = static_cast<double>(i);
                break;
            }
        }
        cycleName = newName;
    } else {
        animationFrame = firstFrame;
        cycleFrame = 0;
        cycleName = newName;
    }
}

// AS: CustomerChar.endAnimationCycle — a cycle that plays once has finished.
void CustomerChar::endAnimationCycle()
{
    events::GameControls &controls = *gameObj->obf_8o;
    if (cycleName == "pickup_map") {
        controls.stopCycle = true;
        // SKELETON: GameDisplay.startMainIrisOut.
    } else if (cycleName == "dead" || cycleName == "crawldead") {
        skidSpeed = 0;
        isSkidding = false;
        isWalking = false;
        isHit = false;
        restartPlayer();
    } else if (cycleName == "hit" || cycleName == "gloves_hit") {
        isHit = false;
        walkingDir = facingDir;
        controls.stopHurtControls = false;
    } else if (cycleName == "skid") {
        isSkidding = false;
    } else if (cycleName == "turnaround" || cycleName == "turncrawl") {
        isTurning = false;
    } else if (cycleName == "unduck") {
        isDucking = false;
        duck = false;
        isCrawling = false;
    } else if (cycleName == "finishpush") {
        isFinishPushing = false;
    } else if (cycleName == "attack1_swing1" || cycleName == "attack2_swing1" || cycleName == "attack1_melee" ||
               cycleName == "attack2_melee" || cycleName == "attack3_melee" || cycleName == "attack4_melee") {
        isAttacking = false;
        // SKELETON: Foodini's balloon effect is reset here.
    } else if (cycleName == "shoot_longgun" || cycleName == "shoot_pistol" || cycleName == "shoot_bazooka") {
        // Guns keep firing while the attack key is held, until the round is spent.
        if (controls.keyPressedAction && bulletsFired < bulletsPerRound) {
            cycleFrame = 0;
            animationFrame = cycleFrames(cycleName + "CycleFrames").frameAt(0);
        } else {
            isAttacking = false;
        }
    }
}

// AS: PlayerChar.addToCombo
void CustomerChar::addToCombo(double amount)
{
    comboTally += amount;
    comboTimer = 0;
    // SKELETON: ChallengeManager.recordCombo(comboTally) when comboTally > 1.
}

// AS: PlayerChar.updateComboTimer
void CustomerChar::updateComboTimer()
{
    if (comboTally > 0) {
        ++comboTimer;
        if (comboTimer >= comboTimerMax) {
            comboTimer = 0;
            comboTally = 0;
        }
    }
}

// AS: CustomerChar.fallChar
void CustomerChar::fallChar()
{
    if (!jump && !isFlying && !isClimbingLadder) {
        getMyCorners(x, y + 1);
        if (!HitDownLeft && !HitDownRight && !checkIfOnCloud() && !checkMovingTiles(1) && onSlope == 0 && !isInQuicksand) {
            jumpspeed = 0;
            jump = true;
            jumpedFromX = x;
            jumpedFromY = y;
        } else if (!duck && !isCrawling) {
            lastSafeTime = gameObj->obf_8o->gameplayTimer;
        }
    }
}

// AS: CustomerChar.checkIfAttacking
void CustomerChar::checkIfAttacking()
{
    const data::CustomerDataFile *data = playerData();
    const std::string &weaponType = data->weaponType;
    if (weaponType == CustomerData::WEAPON_BAZOOKA || weaponType == CustomerData::WEAPON_LONGGUN ||
        weaponType == CustomerData::WEAPON_PISTOL || weaponType == CustomerData::WEAPON_TOSS ||
        data->customerName == "Xandra") {
        if (!canAttack && !isAttacking) {
            ++reloadTimer;
            if (reloadTimer >= reloadTimerMax && !gameObj->obf_8o->keyPressedAction) {
                canAttack = true;
            }
        } else if (isAttacking) {
            // SKELETON: shootWeapon every bulletFrequency frames until bulletsPerRound shots are fired.
            ++bulletTimer;
            if (bulletsFired > 0) {
                ++reloadTimer;
                if (reloadTimer >= reloadTimerMax && !gameObj->obf_8o->keyPressedAction) {
                    canAttack = true;
                }
            }
        }
        if (!isShoved && (!canPressLeft || !canPressRight)) {
            canPressLeft = true;
            canPressRight = true;
        }
    } else {
        if (!canAttack && !isAttacking) {
            canAttack = true;
        } else if (isAttacking && !canAttack && cycleFrame >= repeatAttackFrame) {
            canAttack = true;
        }
        if (!isAttacking) {
            canDealDamage = true;
        }
        if (!isShoved && (!canPressLeft || !canPressRight)) {
            if (jump) {
                canPressLeft = true;
                canPressRight = true;
            } else if (canDealDamage) {
                ++postAttackDelayTimer;
                if (postAttackDelayTimer >= postAttackDelayMax) {
                    canPressLeft = true;
                    canPressRight = true;
                }
            }
        }
        // SKELETON: the whip sound (attack_whip.wav) at startAttackFrame.
    }
}

// AS: CustomerChar.startAttacking
void CustomerChar::startAttacking()
{
    if (isGrabbing || isRidingObject) {
        return;
    }
    const std::string &weaponType = playerData()->weaponType;
    canDealDamage = true;
    isAttacking = true;
    canAttack = false;
    bulletTimer = 0;
    bulletsFired = 0;
    reloadTimer = 0;
    if (weaponType == CustomerData::WEAPON_WHIP || weaponType == CustomerData::WEAPON_TOSS) {
        attackVariation = 1;
    } else if (weaponType == CustomerData::WEAPON_MELEE) {
        if (jump) {
            attackVariation = 4;
        } else {
            // A different punch from the last one, at random.
            std::vector<double> variations = {1, 2, 3, 4};
            for (auto it = variations.begin(); it != variations.end(); ++it) {
                if (*it == attackVariation) {
                    variations.erase(it);
                    break;
                }
            }
            attackVariation = variations[static_cast<size_t>(std::floor(as3::random() * variations.size()))];
        }
    } else if (attackVariation == 1) {
        attackVariation = 2;
    } else {
        attackVariation = 1;
    }
    setupAttackRect();
    isTurning = false;
    walkingDir = facingDir;
    if (jump && speed < 0) {
        speed *= -1;
    }
    isDoubleJumping = false;
    isGliding = false;
    // SKELETON: Foodini's balloon is hidden, and the punch / swing sounds play here.
}

// AS: CustomerChar.setupAttackRect
void CustomerChar::setupAttackRect()
{
    const Rect *rect = &other_attackRect;
    const std::vector<double> *range = &other_frameRange;
    if (hasWeaponType == "swing1" || hasWeaponType == "swing2" || hasWeaponType == "scooter" || hasWeaponType == "kahuna") {
        rect = &swing_attackRect;
        range = &swing_frameRange;
    } else if (hasWeaponType == "melee") {
        rect = &melee_attackRect;
        range = &melee_frameRange;
    } else if (hasWeaponType == "whip") {
        rect = &whip_attackRect;
        range = &whip_frameRange;
    } else if (hasWeaponType == "longgun" || hasWeaponType == "pistol" || hasWeaponType == "bazooka" || hasWeaponType == "toss") {
        reloadTimerMax = 5;
    }
    attackRect = *rect;
    if (range->size() >= 3) {
        startAttackFrame = (*range)[0];
        endAttackFrame = (*range)[1];
        repeatAttackFrame = (*range)[2];
    }
    if (facingDir == -1) {
        attackRect.x = rect->x * -1 - rect->width;
    }
}

// AS: CustomerChar.cancelAttack (PlayerChar.cancelAttack, then Foodini's balloon reset, which is a SKELETON)
void CustomerChar::cancelAttack()
{
    isAttacking = false;
    canAttack = true;
}

// AS: CustomerChar.usingGun
bool CustomerChar::usingGun() const
{
    return hasWeaponType == "longgun" || hasWeaponType == "pistol" || hasWeaponType == "gatling" || hasWeaponType == "bazooka";
}

// AS: CustomerChar.usingPowerTool
bool CustomerChar::usingPowerTool() const
{
    return hasWeaponType == "powertool";
}

// AS: CustomerChar.updateObject
void CustomerChar::updateObject()
{
    events::GameControls &controls = *gameObj->obf_8o;
    const data::CustomerDataFile *data = playerData();
    if (hasWeaponType == "unknown") {
        hasWeaponType = data->weaponType;
    }
    if (duck) {
        collRect = duckCollRect;
    } else if (jump) {
        collRect = jumpCollRect;
    } else {
        collRect = standCollRect;
    }
    // SKELETON: forceGrabYoffset and the boss music delay.
    if (isDoubleJumping && !jump) {
        // SKELETON: Scooter starts sliding here when landing with Down held.
        isDoubleJumping = false;
    }
    updateSprite();
    if (!isDead) {
        // SKELETON: finishing the level (walk to the cage, jump, end portal).
        if (onMovingTile) {
            isSkidding = false;
            skidSpeed = 0;
        }
        if (!isHit && isWalking && !isGrabbing && (isWallJumping || controls.keyPressedLeft || controls.keyPressedRight) &&
            (data->weaponType != CustomerData::WEAPON_MELEE || (!isAttacking || jump))) {
            // Running builds up skid speed, used when the keys are released.
            if (isSkidding) {
                skidTimer = 0;
                skidSpeed = 0;
                isSkidding = false;
            }
            if (std::fmod(skidTimer, skidTimerInterval) == 0 && skidSpeed < maxSkidSpeed) {
                ++skidSpeed;
            }
            if (skidSpeed > std::abs(speed)) {
                skidSpeed = std::abs(speed);
            }
            if (isWallJumping) {
                skidSpeed = walkspeed;
            }
            skidDecayTimer = 0;
            ++skidTimer;
        } else {
            if (!isSkidding && skidSpeed > std::abs(speed)) {
                skidSpeed = std::abs(speed);
            }
            if (skidSpeed > 0 && !jump && !isGrabbing && !isShoved) {
                isSkidding = true;
                ++skidDecayTimer;
                if (cycleName == "stand") {
                    updateSprite();
                }
                double decayInterval = skidDecayTimerInterval;
                if (isHit) {
                    decayInterval = 3;
                }
                if (std::fmod(skidDecayTimer, decayInterval) == 0) {
                    skidSpeed -= skidDecayAmount;
                    if (skidSpeed <= 0) {
                        skidSpeed = 0;
                        skidTimer = 0;
                        isSkidding = false;
                        skidDecayTimer = 0;
                    }
                }
            }
        }
        useSkidSpeed = slopeSpeed(onSlope, skidSpeed, 55);
        if (isHit && !jump && !isSkidding) {
            isHit = false;
            walkingDir = facingDir;
            controls.stopHurtControls = false;
            isStunned = true;
            stunStartTime = 0;
        }
        if (isStunned) {
            ++stunStartTime;
            if (stunStartTime >= stunDuration) {
                isStunned = false;
            }
        }
        if (!isStunned && !isHit && !stunStarIDs.empty()) {
            for (double starID : stunStarIDs) {
                Effect *star = gameObj->obf_V->getEffect(starID);
                if (star && star->effectType == "DizzyEffect") {
                    static_cast<effects::DizzyEffect *>(star)->stopStun();
                }
            }
            stunStarIDs.clear();
        }
        if (isAttackLunging) {
            ++lungeTimer;
            if (std::fmod(lungeTimer, lungeTimerInterval) == 0) {
                --lungeSpeed;
                if (lungeSpeed <= 0) {
                    isAttackLunging = false;
                    lungeTimer = 0;
                    lungeSpeed = 0;
                    useLungeSpeed = 0;
                }
            }
        }
        useLungeSpeed = slopeSpeed(onSlope, lungeSpeed, 55);
        double baseSpeed = baseWalkSpeed;
        if (duck && data->skillType == CustomerData::SKILL_CRAWL) {
            baseSpeed = 5;
        } else if (isInQuicksand) {
            baseSpeed = 5;
        }
        // SKELETON: on a moving tile the base speed is the tile's standingWalkSpeed.
        // Uphill on the steep slopes is slower (67 degrees) than downhill (55).
        double steepAngle = 55;
        if ((onSlope == 7 || onSlope == 8) && walkingDir == 1) {
            steepAngle = 67;
        } else if ((onSlope == 9 || onSlope == 10) && walkingDir == -1) {
            steepAngle = 67;
        }
        walkspeed = slopeSpeed(onSlope, baseSpeed, steepAngle);
        // SKELETON: sliding (slide speed on slopes, Scooter / Kahuna hops, the slide sound), ground pounding,
        // gliding (Foodini's balloons), wall jumping and wall sliding.
        checkIfAttacking();
        checkForHittingEnemies();
        checkForHittingObjects();
        checkForCollectingItems();
        // SKELETON: checkForGrabbingTile, checkSteppingOnObjects,
        // adjustCarriedObject.
        checkIfOnDoor();
        updateStunTime();
        updateComboTimer();
        if (!isGrabbing && !isRidingObject && (!isRidingElevator || isDead)) {
            fallChar();
        }
        if (!jump && !isGrabbing) {
            const managers::MapManager &map = *gameObj->obf_4i;
            if (map.getTileProperty(xtile, ytile + 1, "collision") != 0 ||
                map.getTileProperty(xtile, ytile + 1, "thrublock") != 0) {
                lastSafeX = xtile;
                lastSafeY = ytile;
            }
        }
        // SKELETON: ladder step sounds.
    } else if (!jump) {
        walkingDir = facingDir;
        dirx = facingDir;
        // SKELETON: the end portal opens over the player after falling or crawling to death.
    }
    managers::EffectManager &fx = *gameObj->obf_V;
    if (wasJumping && !jump && !isGrabbing && !isRidingObject && !isInQuicksand) {
        fx.addEffect(x, y + 18, "DustEffect", "anim", false, -1, 0);
        fx.addEffect(x, y + 18, "DustEffect", "anim", false, 1, 0);
        // SKELETON: the jumpland sound.
    }
    if (!wasTurning && isTurning && !jump && !isGrabbing && !isCrawling) {
        if (dirx == -1) {
            fx.addEffect(x + 8, y + 18, "DustEffect", "anim", false, 1, 0);
            fx.addEffect(x + 4, y + 18, "DustEffect", "anim", false, 1, -1);
        } else if (dirx == 1) {
            fx.addEffect(x - 8, y + 18, "DustEffect", "anim", false, -1, 0);
            fx.addEffect(x - 4, y + 18, "DustEffect", "anim", false, -1, -1);
        }
    } else if (isSliding && std::fmod(gameObj->obf_8o->gameplayTimer, 2) == 0) {
        fx.addEffect(x - 6 * facingDir, y + 13, "DustEffect");
    } else if (isAttackLunging && !jump && std::fmod(gameObj->obf_8o->gameplayTimer, 3) == 0 && lungeSpeed > 4) {
        fx.addEffect(x - 6 * facingDir, y + 13, "DustEffect");
    }
    // SKELETON: the turning and sliding sounds.
    wasJumping = jump;
    wasTurning = isTurning;
    playerLastX = x;
    // SKELETON: the crawl hint and Professor Fitz's copter sound.
}

// AS: PlayerChar.clearCombo
void CustomerChar::clearCombo()
{
    // SKELETON: the combo challenge is recorded here when comboTally > 1.
    comboTimer = 0;
    comboTally = 0;
}

// AS: PlayerChar.canGetHit
bool CustomerChar::canGetHit() const
{
    return !isHit && !isStunned && !isInvincible && !isDead && !isFinishingLevel && !isPausingAfterMiniboss &&
           !isHiding && !isRidingObject;
}

// AS: PlayerChar.hurtPlayer — knocked back stunned, or killed when the health runs out.
void CustomerChar::hurtPlayer(double dir, double damage, bool forced)
{
    if (forced && (isPausingAfterMiniboss || isFinishingLevel)) {
        forced = false; // "Don't hurt him, he's finishing the level!"
    }
    if (!canGetHit() && !forced) {
        return;
    }
    events::GameControls &controls = *gameObj->obf_8o;
    currentHealth -= damage;
    isHit = true;
    hitStartTime = 0;
    isGrabbing = false;
    isStartingGrabbing = false;
    isShoved = false;
    clearCombo();
    gameObj->obf_51->gotHurt = true;
    isGroundPounding = false;
    isDoubleJumping = false;
    isClimbingLadder = false;
    isGliding = false;
    isWallJumping = false;
    isWallSliding = false;
    canAutoWallJump = false;
    if (!isCrawling) {
        isDucking = false;
        duck = false;
    }
    // SKELETON: removeCheese when isCheesed.
    if (currentHealth <= 0) {
        currentHealth = 0;
        // SKELETON: GameCam.setCameraJiggle(12, 10).
        cancelAttack();
        dropWeapon();
        dirx = dir;
        if (dir != 0) {
            walkingDir = dir;
            facingDir = dir * -1;
        }
        setSpeed(walkspeed);
        if (!isCrawling) {
            jumpspeed = jumpstart;
            jump = true;
        }
        isDead = true;
        hasPowerup = false;
        isInvincible = false;
        isWalking = false;
        if (invincibleFXID > -1) {
            gameObj->obf_V->removeEffect(invincibleFXID);
            invincibleFXID = -1;
        }
        // SKELETON: player_died.wav.
        controls.stopHurtControls = true;
    } else {
        // SKELETON: GameCam.setCameraJiggle(6, 8).
        dirx = dir;
        if (dir != 0) {
            walkingDir = dir;
            facingDir = dir * -1;
        }
        if (!isCrawling) {
            jumpspeed = -16;
            jump = true;
        }
        isWalking = false;
        setSpeed(8);
        skidSpeed = 3;
        cancelAttack();
        controls.stopHurtControls = true;
        isStunned = true;
        stunStarIDs.push_back(gameObj->obf_V->addEffect(x, y, "DizzyEffect", "faceRight", true, 0, isCrawling ? -32 : -72));
        // SKELETON: player_hurt.wav.
    }
    gameObj->obf_6Z->updatePlayerHealth(currentHealth);
    // SKELETON: throwCarriedObject(0).
}

// AS: PlayerChar.shovePlayer — pushed back without damage.
void CustomerChar::shovePlayer(double dir, double amount)
{
    if (!isGrabbing && !isStartingGrabbing) {
        isShoved = true;
        isTurning = false;
        shoveTimer = 0;
        dirx = dir;
        if (dir != 0) {
            walkingDir = dir;
            facingDir = dir * -1;
        }
        if (dir == 1) {
            canPressLeft = false;
        } else if (dir == -1) {
            canPressRight = false;
        }
        if (jump) {
            amount = 1;
        }
        setSpeed(amount);
    }
}

// AS: PlayerChar.restartPlayer — back to the last safe tile, or out of lives.
void CustomerChar::restartPlayer(double lives)
{
    if (gameObj->obf_51->adjustLives(lives)) {
        gameObj->obf_8o->stopHurtControls = false;
        isDead = false;
        gameObj->obf_8o->resetKeys();
        startupPlayer(lastSafeX, lastSafeY);
    } else {
        // "DEAD!" SKELETON: endMusic and GameDisplay.startMainIrisOut, whose end shows the game over screen
        // (§_-2K§). Until those exist the level starts again.
        gameObj->obf_51->resetLives();
        gameObj->obf_6V(gameObj->obf_AQ->currentLevel);
    }
}

// AS: PlayerChar.updateStunTime
void CustomerChar::updateStunTime()
{
    if (isStunned) {
        ++stunStartTime;
        if (stunStartTime >= stunDuration) {
            isStunned = false;
        }
    }
    if (isShoved) {
        ++shoveTimer;
        if (shoveTimer >= shoveTimerMax) {
            isShoved = false;
            canPressRight = true;
            canPressLeft = true;
            walkingDir = facingDir;
            isWalking = true;
        }
    }
}

// AS: CustomerChar.checkForHittingEnemies — the attack, slide and ground pound hit enemies; stomping stuns them;
// touching one hurts the player (or shoves him away from a harmless one).
bool CustomerChar::checkForHittingEnemies(bool justChecking)
{
    const Rect body{x + collRect.x, y + collRect.y, collRect.width, collRect.height};
    Rect stomp{x + stomp_collRect.x, y + stomp_collRect.y, stomp_collRect.width, stomp_collRect.height};
    const Rect attack{x + getTorsoOffsetX() + attackRect.x, y + getTorsoOffsetY() + attackRect.y, attackRect.width,
                      attackRect.height};
    if (isGroundPounding) {
        stomp = {x + pound_collRect.x, y + pound_collRect.y, pound_collRect.width, pound_collRect.height};
    }
    bool result = false;
    bool hitSomething = false, killedSomething = false, stomped = false;
    const std::string weapon = playerData()->weaponClipName;
    managers::EnemyManager &manager = *gameObj->obf_E;
    for (size_t i = 0; i < manager.enemies.size(); ++i) {
        Enemy *e = manager.enemies[i];
        if (!e) {
            continue;
        }
        if (justChecking) {
            result = false;
            break;
        }
        if (canDealDamage && isAttacking && cycleFrame >= startAttackFrame && cycleFrame <= endAttackFrame) {
            if (e->checkSpriteCollision(attack)) {
                bool killed = false;
                if (!e->isHit) {
                    killed = e->getHit(statAttack, facingDir, false, weapon);
                }
                if (!e->isDodging && e->isHit) {
                    hitSomething = true;
                    if (killed) {
                        killedSomething = true;
                    }
                }
                if (!e->isDead) {
                    if (usingPowerTool()) {
                        if (std::abs(x - e->x) <= 30) {
                            shovePlayer(x < e->x ? -1 : 1);
                        }
                    } else {
                        shovePlayer(x < e->x ? -1 : 1, 0);
                    }
                }
            }
        } else if (isSliding && e->checkSpriteCollision(body) && !e->isDead) {
            e->getHit(statAttack, facingDir, false, playerData()->customerName == "Scooter" ? "slideskate" : "slide");
        } else if (isGroundPounding && e->checkSpriteCollision(stomp) && !e->isDead && e->canBeStunned) {
            e->getHit(statAttack, facingDir, false, "groundpound");
            jumpspeed = -16;
            jump = true;
            isGliding = false;
            isDoubleJumping = false;
            isWallJumping = false;
            isGroundPounding = false;
            // SKELETON: GameCam.setCameraJiggle(10, 10) and papow.wav.
        } else if (canGetHit() && !justChecking) {
            if (e->checkSpriteCollision(body) && !e->isDead) {
                if (isCheesed && jump && jumpspeed >= 0) {
                    // "Hit enemy while cheesed!  Kill it."
                    if (!e->isDead) {
                        e->killMe();
                    }
                    break;
                }
                if (e->canBeStunned && ((jump && jumpspeed >= 0) || (!jump && wasJumping)) &&
                    (!e->hasStompRect || e->checkSpriteCollision(stomp, &e->stompRect))) {
                    e->stunEnemy();
                    stomped = true;
                    gravity = e->stunBounceGravity;
                    jumpspeed = e->stunBounceJumpSpeed;
                    triggerjump = true;
                    jump = true;
                    isGliding = false;
                    isDoubleJumping = false;
                    isWallJumping = false;
                    isWallSliding = false;
                    isGroundPounding = false;
                    // SKELETON: stunbounce.wav at volume 0.7 when the enemy is not dead or hit.
                    break;
                }
                if (!e->isStunned && !e->isUnstunning && !stomped && e->canDealDamage) {
                    hurtPlayer(e->x > x ? -1 : 1, 1);
                    e->reactToHurtingPlayer();
                    break;
                }
                if ((e->isStunned || e->isUnstunning || !e->canDealDamage) && (!jump || jumpspeed >= 0)) {
                    shovePlayer(x < e->x ? -1 : 1, e->shoveAmount);
                    break;
                }
                break;
            }
        }
    }
    if (hitSomething && !usingPowerTool()) {
        canDealDamage = false;
    }
    if (hitSomething && killedSomething) {
        isFinalBlow = true;
    } else if (hitSomething && !killedSomething) {
        isFinalBlow = false;
    } else if (!hitSomething && canDealDamage) {
        isFinalBlow = false;
    }
    return result;
}

// AS: CustomerChar.checkForHittingObjects — the attack breaks objects that accept "punch"; pushing.
bool CustomerChar::checkForHittingObjects()
{
    const Rect attack{x + getTorsoOffsetX() + attackRect.x, y + getTorsoOffsetY() + attackRect.y, attackRect.width,
                      attackRect.height};
    bool hit = false, pushing = false;
    managers::ObjectManager &manager = *gameObj->obf_7u;
    for (size_t i = 0; i < manager.objects.size(); ++i) {
        GameObject *o = manager.objects[i];
        if (!o) {
            continue;
        }
        if (canDealDamage && isAttacking && cycleFrame >= startAttackFrame && cycleFrame <= endAttackFrame) {
            if (o->checkSpriteCollision(attack) && o->acceptsDamage("weapon", "punch")) {
                o->getHit();
                hit = true;
                // AS: it then computes a random spot above the object that nothing uses.
            }
        }
        if (i < manager.objects.size() && manager.objects[i] == o && o->isPushable && o->checkPushingObject()) {
            pushing = true;
        }
    }
    if (hit && !usingPowerTool()) {
        canDealDamage = false;
    }
    if (pushing && !jump && !isAttacking && !isHit && !duck) {
        isFinishPushing = false;
        isPushingObject = true;
        wasPushing = true;
    } else {
        isPushingObject = false;
        if (wasPushing) {
            isFinishPushing = true;
            wasPushing = false;
        }
    }
    return hit;
}

// AS: CustomerChar.checkForPoundingObjects
void CustomerChar::checkForPoundingObjects()
{
    const Rect pound{x + pound_collRect.x, y + pound_collRect.y, pound_collRect.width, pound_collRect.height};
    if (isGroundPounding) {
        managers::ObjectManager &manager = *gameObj->obf_7u;
        for (int i = static_cast<int>(manager.objects.size()) - 1; i >= 0; --i) {
            GameObject *o = i < static_cast<int>(manager.objects.size()) ? manager.objects[static_cast<size_t>(i)] : nullptr;
            if (o && o->checkSpriteCollision(pound) && o->acceptsDamage("weapon", "Pound")) {
                o->getHit();
            }
        }
    }
}

// AS: PlayerChar.getHeart
void CustomerChar::getHeart()
{
    if (currentHealth < maxHealth) {
        ++currentHealth;
    }
    gameObj->obf_6Z->updatePlayerHealth(currentHealth);
}

// AS: PlayerChar.checkForCollectingItems
void CustomerChar::checkForCollectingItems()
{
    gameObj->obf_3Z->checkForCollecting({x + collRect.x, y + collRect.y, collRect.width, collRect.height});
}

} // namespace papaGame::models::characters
