#include "game/papaGame/models/characters/CustomerChar.h"

#include <algorithm>
#include <climits>
#include <cmath>

#include "engine/json.h"
#include "game/as3.h"
#include "game/obf_9G/obf_4R.h"
#include "game/obf_AF/obf_1n.h"
#include "game/papaGame/models/effects/DizzyEffect.h"
#include "game/papaGame/models/effects/EndPortalEffect.h"

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
        skillType = c->skillType;
        hasWeaponType = c->weaponType;
        if (skillType == CustomerData::SKILL_CRAWL) {
            playerheight = 50;
        }
        canCrawl = skillType == CustomerData::SKILL_CRAWL;
        canPush = skillType == CustomerData::SKILL_PUSH;
        // SKELETON: BulletManager.addToPreblit(bulletName) for the customers with a projectile.
        jumpstart = c->jumpstart;
        jumpgravity = c->jumpgravity;
        baseWalkSpeed = c->walkspeed;
        walkspeed = baseWalkSpeed;
        maxSkidSpeed = c->maxskidspeed;
        skidDecayTimerInterval = c->skiddecay;
        skidDecayAmount = c->skiddecayamount;
        bulletFrequency = c->bulletstart;
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
    if (onSlope > 0 && (skillType != CustomerData::SKILL_CRAWL || !isCrawling)) {
        startSliding(1);
    } else {
        duck = true;
        isDucking = true;
        isTurning = false;
    }
}

// AS: PlayerChar.startSliding — downhill on a slope, or straight ahead.
void CustomerChar::startSliding(double startSpeed, bool useSlope)
{
    if (isSliding) {
        return;
    }
    if (onSlope != 0 && useSlope) {
        const double dir = (onSlope == 1 || onSlope == 3 || onSlope == 4 || onSlope == 7 || onSlope == 8) ? -1 : 1;
        facingDir = dir;
        walkingDir = dir;
        dirx = dir;
    } else {
        walkingDir = facingDir;
        dirx = walkingDir;
    }
    isSliding = true;
    slideSpeed = startSpeed;
    baseSlideSpeed = startSpeed;
    if (!slideSound) {
        if (playerData()->customerName == "Scooter") {
            slideSound = gameObj->obf_7C->playSound("skateboard_slide.wav", true, 0.1);
        } else {
            slideSound = gameObj->obf_7C->playSound("hill_slide.wav", true, 0.1);
        }
    }
}

// AS: PlayerChar.startPounding — SKELETON: the "groundpound" / "poundcontext" tutorials (§_-c§).
void CustomerChar::startPounding()
{
    if (skillType == CustomerData::SKILL_POUND && !isGroundPounding) {
        isGroundPounding = true;
        jump = true;
        jumpspeed = 0;
    }
}

// AS: PlayerChar.startDoubleJumping — SKELETON: the "doublejump" tutorial (§_-c§).
void CustomerChar::startDoubleJumping()
{
    if (skillType == CustomerData::SKILL_DOUBLEJUMP && !isDoubleJumping) {
        isDoubleJumping = true;
        jump = true;
        jumpspeed = jumpstart;
        gravity = jumpgravity;
    }
}

// AS: PlayerChar.startGliding — SKELETON: the "glide" tutorial (§_-c§).
void CustomerChar::startGliding()
{
    if (skillType == CustomerData::SKILL_GLIDE && jump && !isGliding) {
        isGliding = true;
        gravity = glidegravity;
        if (canGlideBoost) {
            jumpspeed = -6;
            canGlideBoost = false;
        } else {
            jumpspeed = 0;
        }
        if (playerData()->customerName == "Foodini") {
            gameObj->obf_7C->playSound("balloon_glide.wav");
        } else if (playerData()->customerName != "Professor Fitz") {
            gameObj->obf_7C->playSound("hatglider.wav", false, 0.3);
        }
    }
}

// AS: PlayerChar.startWallJumping — away from the wall. SKELETON: the "walljump" tutorial (§_-c§).
void CustomerChar::startWallJumping(double dir, const std::string &key)
{
    isWallJumping = true;
    isWallSliding = false;
    jumpspeed = jumpstart;
    gravity = jumpgravity;
    lastWallJumpKey = key;
    lastWallJumpOtherKey = key;
    canPressWallJump = false;
    canArrowWallJump = false;
    speed = 10;
    canAutoWallJump = true;
    if (checkIfAgainstWall(1) || dir == -1) {
        facingDir = -1;
        walkingDir = -1;
        dirx = -1;
        triggerjump = true;
        triggerjumpx = true;
        lastWallJumpKey = "right";
    } else if (checkIfAgainstWall(-1) || dir == 1) {
        facingDir = 1;
        walkingDir = 1;
        dirx = 1;
        triggerjump = true;
        triggerjumpx = true;
        lastWallJumpKey = "left";
    }
}

// AS: CustomerChar.stopGrabRiding — letting go of a moving rideable launches the player along it.
void CustomerChar::stopGrabRiding()
{
    if (!isRidingObject) {
        return;
    }
    double dir = 0;
    if (whichObjectRiding) {
        dir = whichObjectRiding->getRideableDirection();
        whichObjectRiding->releaseRideableObject();
        whichObjectRiding = nullptr;
    }
    isRidingObject = false;
    if (dir != 0) {
        walkingDir = facingDir;
        dirx = walkingDir;
        isSkidding = false;
        skidSpeed = walkspeed * 2;
        speed = walkspeed * 2;
        isWalking = true;
        jump = true;
    }
}

// AS: CustomerChar.checkForCrawlGap — a crawler walking into a low gap ducks into it (corners already read).
bool CustomerChar::checkForCrawlGap(double dir)
{
    bool found = false;
    std::string how;
    if (skillType == CustomerData::SKILL_CRAWL && !jump && !isAttacking) {
        if (dir == -1 && !HitDownLeft && (HitUpLeft || HitUpMidLeft || HitDownMidLeft)) {
            how = "left";
        } else if (dir == 1 && !HitDownRight && (HitUpRight || HitUpMidRight || HitDownMidRight)) {
            how = "right";
        } else if (onSlope == 9 && dir == -1 && !HitDownMidLeft && (HitUpMidLeft || HitUpLeft)) {
            how = "9-1";
        } else if (onSlope == 8 && dir == 1 && !HitDownMidRight && (HitUpMidRight || HitUpRight)) {
            how = "8-1";
        } else if (onSlope == 9 && dir == -1 && !HitDownMidLeft && onSlopeWallLeft) {
            how = "9-2";
        } else if (onSlope == 8 && dir == 1 && !HitDownMidRight && onSlopeWallRight) {
            how = "8-2";
        }
        if (!how.empty()) {
            found = true;
            duck = true;
            isDucking = true;
            isTurning = false;
        }
    }
    if (found) {
        SDL_Log("Start Crawling into gap! (%s)", how.c_str());
    }
    return found;
}

// AS: CustomerChar.isInsideCrawlGap — something solid above a ducking player.
bool CustomerChar::isInsideCrawlGap()
{
    if (!duck) {
        return false;
    }
    const managers::MapManager &map = *gameObj->obf_4i;
    getMyCorners(x, y);
    auto either = [&](double row, const char *prop) {
        return map.getTileProperty(leftX, row, prop) > 0 || map.getTileProperty(rightX, row, prop) > 0;
    };
    return either(standupY, "collision") || either(standupmidY, "collision") || either(upY, "collision") ||
           (onSlope == 9 && facingDir == -1 && map.getTileProperty(xtile - 1, ytile - 2, "collision") != 0) ||
           (onSlope == 8 && facingDir == 1 && map.getTileProperty(xtile + 1, ytile - 2, "collision") != 0) ||
           either(standupY, "ceilingslope") || either(standupmidY, "ceilingslope") || either(upY, "ceilingslope");
}

// AS: CustomerChar.checkIfAgainstWall — only wall jumpers ever are.
bool CustomerChar::checkIfAgainstWall(double dir)
{
    bool against = false;
    if (skillType == CustomerData::SKILL_WALLJUMP) {
        if (dir == 0 || dir == -1) {
            getMyCorners(x - 6, y);
            if ((HitDownLeft || HitMidLeft) && HitUpLeft) {
                against = true;
            }
        }
        if (dir == 0 || dir == 1) {
            getMyCorners(x + 6, y);
            if ((HitDownRight || HitMidRight) && HitUpRight) {
                against = true;
            }
        }
    }
    return against;
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
        if (isAttacking && hasWeaponType == "whip" && cycleFrame == startAttackFrame) {
            gameObj->obf_7C->playSound("attack_whip.wav");
        }
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
    // SKELETON: Foodini's balloon (balloonFXID) is hidden here.
    if (weaponType == CustomerData::WEAPON_MELEE || weaponType == CustomerData::WEAPON_TOSS) {
        gameObj->obf_7C->playSound("attack_punch_" + as3::String(std::min(2.0, attackVariation)) + ".wav");
    } else if (hasWeaponType == "swing1" || hasWeaponType == "swing2" || hasWeaponType == "kahuna" ||
               hasWeaponType == "scooter") {
        gameObj->obf_7C->playSound("attack_swing_" + as3::String(std::min(2.0, attackVariation)) + ".wav");
        if (playerData()->customerName == "Xandra") {
            gameObj->obf_7C->playSound("xandra_sparkle.wav");
        }
    }
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
        if (data->customerName == "Scooter" && (isWalking || isSkidding || onSlope != 0) && speed > 0 &&
            controls.keyPressedDown) {
            startSliding(speed, false);
        }
        isDoubleJumping = false;
    }
    updateSprite();
    if (!isDead) {
        if (isFinishingLevel) {
            isSkidding = false;
            skidSpeed = 0;
            if (finishType == "cage") {
                managers::EffectManager &fx = *gameObj->obf_V;
                if (!jump && !isRidingObject && x != finishTargetX) {
                    // Walk to the spot beside the cage.
                    isWalking = true;
                    speed = walkspeed;
                    const double dir = finishTargetX > x ? 1 : -1;
                    facingDir = dir;
                    walkingDir = dir;
                    dirx = dir;
                    moveChar(dirx, 0, 0);
                    if ((dir == 1 && x > finishTargetX) || (dir == -1 && x < finishTargetX)) {
                        x = finishTargetX;
                    }
                } else if (x == finishTargetX || finishAddedPortal) {
                    isWalking = false;
                    if (finishJumps < finishJumpsMax) {
                        // Jump for joy (the freed customer jumps too).
                        if (!jump) {
                            ++finishJumps;
                            jumpspeed = jumpstart / 2;
                            jump = true;
                        } else if (jump && jumpspeed >= 0) {
                            if (Effect *customer = fx.getEffect(finishCustomerFXIndex)) {
                                if (!customer->jump) {
                                    customer->jump = true;
                                    customer->jumpspeed = jumpstart / 2;
                                }
                            }
                        }
                    } else if (!jump && !finishAddedPortal) {
                        finishPortalID = fx.addEffect(x + 54 * facingDir, y - 54, "EndPortalEffect");
                        finishAddedPortal = true;
                        gameObj->obf_7C->playSound("portalsound_spin.wav");
                    } else if (finishAddedPortal) {
                        ++finishWaitTimer;
                        Effect *portal = fx.getEffect(finishPortalID);
                        if (finishWaitTimer == finishWaitTimerMax - 1) {
                            if (portal) {
                                gameObj->obf_71->startMainIrisOut(portal->x, portal->y);
                            }
                        } else if (finishWaitTimer == finishWaitTimerMax) {
                            // SKELETON: the CageCustomerEffect heads for the portal too (portalX / portalY).
                            gravity = 0;
                            jumpgravity = 0;
                            normalgravity = 0;
                            isWalking = false;
                            isSkidding = false;
                            speed = 0;
                            walkspeed = 0;
                            isTurning = false;
                            jump = false;
                        } else if (finishWaitTimer > finishWaitTimerMax && portal) {
                            // Fly into the portal.
                            jump = true;
                            jumpspeed = 0;
                            const double distx = portal->x - x, disty = portal->y - y;
                            x += distx / 3;
                            y += disty / 3;
                            if (std::abs(distx) <= 4 && std::abs(disty) <= 4 && !isGone) {
                                isGone = true;
                                static_cast<effects::EndPortalEffect *>(portal)->isClosed = true;
                                gameObj->obf_71->flashPortal = true;
                                gameObj->obf_7C->playSound("portalsound_spinfallout.wav");
                                fx.removeEffect(finishCustomerFXIndex);
                            }
                        }
                    }
                }
            }
            // SKELETON: the "fake" finish (boss levels) and the other finish types.
        }
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
        const bool hopper = data->customerName == "Scooter" || data->customerName == "Kahuna";
        // Sliding stops in a hop (Scooter, Kahuna) or a crouch.
        auto stopSliding = [&] {
            isSliding = false;
            baseSlideSpeed = 0;
            slideSpeed = 0;
            if (hopper) {
                jump = true;
                jumpspeed = -12;
                if (data->customerName == "Scooter") {
                    gameObj->obf_7C->playSound("skateboard_hop.wav");
                }
            } else {
                isDucking = true;
            }
        };
        if (isSliding) {
            walkingDir = facingDir;
            dirx = facingDir;
            const double timer = controls.gameplayTimer;
            if (jump) {
                isSliding = false;
                skidSpeed = slideSpeed;
                if (data->customerName == "Scooter") {
                    gameObj->obf_7C->playSound("skateboard_hop.wav");
                }
            } else if (onSlope > 0) {
                if (baseSlideSpeed == 0 || std::fmod(timer, slideDecayTimerInterval) == 0) {
                    // Faster downhill, slower uphill; the shallow slopes (3-6) change by 1, the others by 2.
                    auto alongSlope = [&](bool downhill, bool shallow) {
                        if (downhill) {
                            if (baseSlideSpeed < maxSlideSpeed) {
                                baseSlideSpeed += shallow ? 1 : 2;
                            }
                            if (baseSlideSpeed > maxSlideSpeed) {
                                baseSlideSpeed = maxSlideSpeed;
                            }
                        } else if (baseSlideSpeed >= 0) {
                            if (data->customerName == "Scooter") {
                                if (shallow) {
                                    if (std::fmod(timer, 2) == 0) {
                                        --baseSlideSpeed;
                                    }
                                } else {
                                    --baseSlideSpeed;
                                }
                            } else if (shallow) {
                                --baseSlideSpeed;
                            } else {
                                baseSlideSpeed -= 2;
                            }
                        }
                    };
                    if (baseSlideSpeed < 0) {
                        stopSliding();
                    } else if (onSlope == 1 || onSlope == 3 || onSlope == 4 || onSlope == 7 || onSlope == 8) {
                        alongSlope(facingDir == -1, onSlope == 3 || onSlope == 4);
                    } else if (onSlope == 2 || onSlope == 5 || onSlope == 6 || onSlope == 9 || onSlope == 10) {
                        alongSlope(facingDir == 1, onSlope == 5 || onSlope == 6);
                    }
                }
            } else if (baseSlideSpeed > 0) {
                if (data->customerName == "Scooter") {
                    if (std::fmod(timer, slideDecayTimerInterval * 2) == 0) {
                        --baseSlideSpeed;
                    }
                } else if (std::fmod(timer, slideDecayTimerInterval) == 0) {
                    --baseSlideSpeed;
                }
            } else if (onSlope == 0 && baseSlideSpeed <= 0 && !isSkidding) {
                stopSliding();
            }
        } else {
            baseSlideSpeed = 0;
            slideSpeed = 0;
        }
        if (!isSliding && !isWallSliding && slideSound) {
            slideSound->stop();
            slideSound = nullptr;
        } else if (isSliding && slideSound) {
            double volume = baseSlideSpeed / maxSlideSpeed;
            if (data->customerName == "Scooter") {
                volume *= 2;
                if (volume > 1) {
                    volume = 1;
                }
            }
            slideSound->setTransform(volume, 0);
        } else if (isWallSliding && slideSound) {
            if (std::fmod(controls.gameplayTimer, 3) == 0) {
                double volume = slideSound->volume() + 0.1;
                if (volume > 1) {
                    volume = 1;
                }
                slideSound->setTransform(volume, 0);
            }
        } else if (isWallSliding && !slideSound) {
            slideSound = gameObj->obf_7C->playSound("hill_slide.wav", true, 0.1);
        }
        if (baseSlideSpeed > 0) {
            slideSpeed = slopeSpeed(onSlope, baseSlideSpeed, steepAngle);
        }
        if (isGroundPounding) {
            checkForPoundingObjects();
            fallChar();
            if (!poundSound) {
                poundSound = gameObj->obf_7C->playSound("groundpound_dive.wav", true);
                gameObj->obf_7C->playSound("groundpound_start.wav");
            }
            if (!jump) {
                isGroundPounding = false;
                jumpspeed = jumpstart / 2;
                jump = true;
                // SKELETON: GameCam.setCameraJiggle(10, 10).
                gameObj->obf_7C->playSound("papow.wav");
            }
        } else if (poundSound) {
            poundSound->stop();
            poundSound = nullptr;
        }
        if (isGliding) {
            // SKELETON: Foodini's six FoodiniGlideEffect balloons (foodiniGlideBalloonIDs) and floating them away.
            if (!jump) {
                isGliding = false;
            }
        }
        if (!canGlideBoost && !jump) {
            canGlideBoost = true;
        }
        if (isWallJumping) {
            if (!jump || jumpspeed > 0) {
                isWallJumping = false;
                triggerjump = false;
                triggerjumpx = false;
                speed = walkspeed;
            }
            isWallSliding = false;
        } else if (onSlope == 0 && jump && !isCheesed && !isInQuicksand && !isAttacking && !controls.keyPressedDown &&
                   jumpspeed >= 0 &&
                   (((controls.keyPressedRight || canAutoWallJump) && facingDir == 1 && checkIfAgainstWall(1)) ||
                    ((controls.keyPressedLeft || canAutoWallJump) && facingDir == -1 && checkIfAgainstWall(-1)))) {
            ++wallSlideStartTimer;
            if (canAutoWallJump || wallSlideStartTimer >= wallSlideStartTimerMax) {
                if (!isWallSliding) {
                    jumpspeed = 1;
                }
                isWallSliding = true;
                wallSlideCancelTimer = 0;
            }
        } else if (isWallSliding && (controls.keyPressedDown || !jump || onSlope > 0 ||
                                     (facingDir == 1 && !checkIfAgainstWall(1)) ||
                                     (facingDir == -1 && !checkIfAgainstWall(-1)))) {
            isWallSliding = false;
            canAutoWallJump = false;
            jumpspeed = 1;
        } else if (isWallSliding &&
                   ((checkIfAgainstWall(1) && facingDir == 1 && !canAutoWallJump && !controls.keyPressedRight) ||
                    (checkIfAgainstWall(-1) && facingDir == -1 && !canAutoWallJump && !controls.keyPressedLeft))) {
            ++wallSlideCancelTimer;
            if (wallSlideCancelTimer >= wallSlideCancelTimerMax) {
                isWallSliding = false;
                canAutoWallJump = false;
                jumpspeed = 1;
            }
        }
        if (!jump && !isWallSliding && canAutoWallJump) {
            canAutoWallJump = false;
        }
        if (!jump || !checkIfAgainstWall()) {
            wallSlideStartTimer = 0;
        }
        checkIfAttacking();
        checkForHittingEnemies();
        checkForHittingObjects();
        checkForCollectingItems();
        checkForGrabbingTile();
        // SKELETON: checkSteppingOnObjects,
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
        if (isClimbingLadder && climbDir != 0 && cycleFrame == 1) {
            gameObj->obf_7C->playSound("ladder_step1.wav");
        } else if (isClimbingLadder && climbDir != 0 && cycleFrame == 9) {
            gameObj->obf_7C->playSound("ladder_step2.wav");
        }
    } else if (!jump) {
        walkingDir = facingDir;
        dirx = facingDir;
        // SKELETON: the end portal opens over the player after falling or crawling to death.
    }
    managers::EffectManager &fx = *gameObj->obf_V;
    if (wasJumping && !jump && !isGrabbing && !isRidingObject && !isInQuicksand) {
        fx.addEffect(x, y + 18, "DustEffect", "anim", false, -1, 0);
        fx.addEffect(x, y + 18, "DustEffect", "anim", false, 1, 0);
        gameObj->obf_7C->playSound("jumpland");
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
        gameObj->obf_7C->playSound("player_died.wav");
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
        gameObj->obf_7C->playSound("player_hurt.wav");
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
        // "DEAD!" The analytics event is dropped with the Flipline API.
        gameObj->obf_7C->endMusic();
        gameObj->obf_8o->stopCycle = true;
        gameObj->obf_71->startMainIrisOut();
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
            // SKELETON: GameCam.setCameraJiggle(10, 10).
            gameObj->obf_7C->playSound("papow.wav");
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
                    if (!e->isDead && !e->isHit) {
                        gameObj->obf_7C->playSound("stunbounce.wav", false, 0.7);
                    }
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

// AS: CustomerChar.checkForGrabbingTile — catching a rideable object (the cage pulley) while falling.
// SKELETON: grabbing "grab" tiles (hanging from bars), which needs the hanging movement in keyCheckPlayer.
void CustomerChar::checkForGrabbingTile()
{
    events::GameControls &controls = *gameObj->obf_8o;
    if (!(jump && jumpspeed >= 0 && !isGrabbing && !isRidingObject && !isHit && !isShoved && !isCarryingObject &&
          !controls.keyPressedDown)) {
        return;
    }
    const managers::MapManager &map = *gameObj->obf_4i;
    const double left = map.getTileProperty(leftX, ytile + grabTileYOffset, "grab");
    const double right = map.getTileProperty(rightX, ytile + grabTileYOffset, "grab");
    if (left > 0 || right > 0) {
        return; // SKELETON: tile grabbing
    }
    const Rect reach{x - 20, y - 80, 40, 40};
    for (GameObject *o : gameObj->obf_7u->objects) {
        if (o && o->checkOnScreen() && o->isGrabRideable && !o->isGrabRiding && !o->delayRideableUntilPickup &&
            o->checkSpriteCollision(reach)) {
            isRidingObject = true;
            whichObjectRiding = o;
            skidSpeed = 0;
            jump = false;
            const double dir = o->getRideableDirection();
            ridingObjectDirX = dir;
            if (dir != 0) {
                facingDir = dir;
                walkingDir = facingDir;
                dirx = facingDir;
            }
            cancelAttack();
            o->grabRideableObject(this);
            break;
        }
    }
}

// AS: PlayerChar.finishLevel — the controls stop and the finish sequence starts (updateObject).
void CustomerChar::finishLevel(const std::string &how)
{
    gameObj->obf_8o->stopActionControls = true;
    isFinishingLevel = true;
    isWalking = false;
    finishType = how;
    isGliding = false;
    isGroundPounding = false;
    if (how != "fake") {
        gameObj->obf_7C->playTrack("", INT_MAX);
        gameObj->obf_7C->playSound("papalouie2_fanfare.wav");
    }
    // SKELETON: ChallengeManager.recordCompletionTime / checkCustomChallenges when the finish is not "fake".
    SDL_Log("-- FINISH LEVEL: %s", how.c_str());
}

} // namespace papaGame::models::characters
