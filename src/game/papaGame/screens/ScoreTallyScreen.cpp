#include "game/papaGame/screens/ScoreTallyScreen.h"

#include <SDL3/SDL.h>

#include <cmath>
#include <vector>

#include "game/as3.h"
#include "game/obf_9G/obf_4R.h"
#include "game/obf_AF/obf_81.h"

namespace papaGame::screens {

using data::CustomerData;
using obf_AF::obf_81;

// AS: ScoreTallyScreen.ScoreTallyScreen
ScoreTallyScreen::ScoreTallyScreen(obf_9G::obf_4R *gameObj_, engine::MovieClip *container_)
    : gameObj(gameObj_), container(container_)
{
    fromGameOver = false;
    setupScreen();
}

double &ScoreTallyScreen::tallyValue(const std::string &label)
{
    if (label == "points") {
        return tally_points;
    }
    if (label == "timepoints") {
        return tally_timepoints;
    }
    if (label == "challenge") {
        return tally_challenge;
    }
    if (label == "levelpoints") {
        return tally_levelpoints;
    }
    if (label == "high") {
        return tally_high;
    }
    if (label == "totalpoints") {
        return tally_totalpoints;
    }
    if (label == "money") {
        return tally_money;
    }
    return tally_keys; // no other label is tallied
}

// AS: ScoreTallyScreen.setupScreen — the analytics calls (InitialTime / InitialExcessiveTime) and the API's
// leaderboard reset are left out. The bar's events come through the game's container.
void ScoreTallyScreen::setupScreen()
{
    data::UserData &user = *gameObj->obf_51;
    const double level = gameObj->obf_AQ->currentLevel;
    clip = &container->addChild(engine::MovieClip::create(*gameObj->symbols, "scoreTallyMC"), "");
    setupCharacters();
    if (engine::MovieClip *cover = clip->findChild("leaderboard_cover")) {
        cover->setVisible(false);
    }
    irisClip = &container->addChild(engine::MovieClip::create(*gameObj->symbols, "irisTransitionMC"), "");
    irisClip->setPosition(30, -30);
    clip->setText("title_txt", gameObj->obf_AQ->getWorldTitle(level));
    clip->setText("levelnum_txt", level == 9 ? "X" : as3::String(level + 1));
    if (engine::MovieClip *levelinside = clip->findChild("levelinside")) {
        if (engine::MovieClip *inside = levelinside->findChild("inside")) {
            inside->gotoAndStop(static_cast<int>(level) + 1);
        }
        levelinside->setMask(clip->findChild("levelmask"));
    }
    clip->setText("time_txt", obf_81::obf_2i(gameObj->obf_8o->gameplayTimer));
    getScoreValues();
    clip->setText("points_txt", "");
    clip->setText("timepoints_txt", "");
    clip->setText("challenge_txt", "");
    clip->setText("money_txt", "");
    clip->setText("totalmoney_txt", obf_81::obf_7w(tally_totalmoney_previous));
    clip->setText("levelpoints_txt", "");
    clip->setText("high_txt", obf_81::obf_7w(tally_high_previous));
    clip->setText("totalpoints_txt", obf_81::obf_7w(tally_total_previous));
    if (engine::MovieClip *flasher = clip->findChild("highscore_flasher")) {
        flasher->setVisible(false);
        flasher->gotoAndStop(1);
    }
    clip->setText("totalwarpcoins_txt", as3::String(user.getWarpCoins()) + "/50");
    gameObj->addContainerListener("clickContinue", [this] { clickContinue(); });
    gameObj->addContainerListener("clickQuit", [this] { clickQuit(); });
    for (int i = 1; i <= 6; ++i) {
        engine::MovieClip *icon = clip->findChild("icon" + std::to_string(i));
        engine::MovieClip *panel = clip->findChild("panel" + std::to_string(i));
        if (!icon || !panel) {
            continue;
        }
        icon->gotoAndStop(user.hasCompletedChallenge(level, i) ? 2 : 1);
        icon->setText("num_txt", std::to_string(i));
        const std::string type = gameObj->obf_94->getChallengeType(level, i);
        const std::string target = gameObj->obf_94->getChallengeTargetAmount(level, i);
        const std::string skill = gameObj->obf_94->getChallengeSkillNeeded(level, i);
        engine::MovieClip *panelIcon = panel->findChild("icon");
        if (type == data::Challenge::RESCUE) {
            panel->setText("description_txt", gameObj->lang->get("challenge.rescue"));
            const double trapped = gameObj->obf_3A->getTrappedCustomerIndex(level, i);
            engine::MovieClip::BitmapChild face = gameObj->obf_3A->getCustomerBitmap(trapped);
            face.x = -8;
            face.y = -8;
            const std::string name = gameObj->obf_3A->getCustomerName(trapped);
            if (name == "Georgito" || name == "Yippy" || name == "Greg") {
                face.y -= 8;
            }
            if (panelIcon) {
                panelIcon->gotoAndStop(1);
                if (engine::MovieClip *holder = panelIcon->findChild("holder")) {
                    holder->addBitmap(face);
                    holder->setMask(panelIcon->findChild("masker"));
                }
            }
        } else if (type == data::Challenge::BURGERZILLAS) {
            panel->setText("description_txt", gameObj->lang->get("challenge.defeat", target));
            if (panelIcon) {
                panelIcon->gotoAndStop(2);
            }
        } else if (type == data::Challenge::COINS) {
            panel->setText("description_txt", gameObj->lang->get("challenge.find", target));
            if (panelIcon) {
                panelIcon->gotoAndStop(3);
            }
        } else {
            panel->setText("description_txt", gameObj->lang->get("challenge.find", target));
            if (panelIcon) {
                panelIcon->gotoAndStop(4 + static_cast<int>(level));
            }
        }
        engine::MovieClip *skillClip = panel->findChild("skill");
        if (skill == CustomerData::SKILL_NONE || skill.empty()) {
            if (skillClip) {
                skillClip->stop();
                skillClip->setVisible(false);
            }
            if (engine::MovieClip *needs = panel->findChild("needs")) {
                needs->setVisible(false);
            }
        } else if (skillClip) {
            skillClip->gotoLabel(skill, false);
        }
        if (type.empty()) {
            panel->setVisible(false);
            icon->setVisible(false);
        }
    }
    gameObj->obf_7C->playTrack("AlternateTrack", 1, 0, "outin");
    irisClip->gotoLabel("irisin", true);
    gameObj->obf_7C->playSound("portalsound_spinfallout.wav");
}

// AS: ScoreTallyScreen.getScoreValues — reading the values also records the level in the save slot.
void ScoreTallyScreen::getScoreValues()
{
    data::UserData &user = *gameObj->obf_51;
    tally_challenge = user.getChallengeBonus();
    tally_timepoints = user.getTimeBonus();
    tally_money = user.getCurrentMoney();
    tally_points = user.getCurrentPoints();
    tally_high_previous = user.getLevelHighScore();
    tally_total_previous = user.getTotalScore();
    tally_totalmoney_previous = user.getTotalMoney();
    user.unlockNextLevel();
    user.setLevelTime(gameObj->obf_8o->gameplayTimer);
    tally_levelpoints = user.updateAndGetLevelScore();
    tally_high = user.getLevelHighScore();
    tally_totalpoints = user.getTotalScore();
    tally_totalmoney = user.getTotalMoney();
    isNewHighScore = tally_levelpoints > tally_high_previous;
}

// AS: ScoreTallyScreen.nextTally
void ScoreTallyScreen::nextTally()
{
    delayTimer = 0 - delayBetweenReveals;
    ++whichTallyIndex;
    if (whichTallyIndex < static_cast<double>(std::size(tallyLabels))) {
        revealingWhich = tallyLabels[static_cast<size_t>(whichTallyIndex)];
        if (revealingWhich == "high" && !isNewHighScore) {
            SDL_Log("Skip high score and total tally, go right to money.");
            revealingWhich = "money";
        }
    } else {
        revealingWhich = "done";
    }
}

// AS: ScoreTallyScreen.tallyScore — one value counts up from 0 (or from its previous value) over the delay.
void ScoreTallyScreen::tallyScore()
{
    if (checkAndIncrement()) {
        clip->setText(revealingWhich + "_txt", obf_81::obf_7w(tallyValue(revealingWhich)));
        if (revealingWhich == "money") {
            clip->setText("totalmoney_txt", obf_81::obf_7w(tally_totalmoney));
        }
        nextTally();
    } else if (delayTimer > 0) {
        const double f = delayTimer / delayBetweenReveals;
        std::string text = "0";
        if (revealingWhich == "high") {
            text = obf_81::obf_7w(std::round(tally_high_previous + (tally_high - tally_high_previous) * f));
            if (isNewHighScore) {
                engine::MovieClip *flasher = clip->findChild("highscore_flasher");
                if (flasher && !flasher->visible()) {
                    flasher->setVisible(true);
                    flasher->gotoAndPlay(1);
                }
            }
        } else if (revealingWhich == "totalpoints") {
            text = obf_81::obf_7w(std::round(tally_total_previous + (tally_totalpoints - tally_total_previous) * f));
        } else {
            text = obf_81::obf_7w(std::round(tallyValue(revealingWhich) * f));
            if (revealingWhich == "money") {
                clip->setText("totalmoney_txt",
                              obf_81::obf_7w(std::round(tally_totalmoney_previous + tally_money * f)));
            }
        }
        clip->setText(revealingWhich + "_txt", text);
    }
}

// AS: ScoreTallyScreen.updateScreen (ENTER_FRAME)
void ScoreTallyScreen::updateScreen()
{
    updateCharacters();
    if (animatingIn) {
        if (irisClip->currentFrameLabel() == "stopirisout") {
            clip->setVisible(true);
            animatingIn = false;
            gameObj->obf_7C->playTrack("AlternateTrack");
            irisClip->gotoLabel("irisin", true);
        }
    } else if (animatingOut) {
        if (irisClip->currentFrameLabel() == "stopirisout") {
            animatingOut = false;
            scoreTallyClosed();
        }
    } else if (revealingWhich == "anim") {
        // (No tally label is "anim", and scoreTallyMC has no tallyclip: kept as the original has it.)
        if (checkAndIncrement()) {
            nextTally();
        } else if (delayTimer == 1) {
            if (engine::MovieClip *tallyclip = clip->findChild("tallyclip")) {
                tallyclip->gotoLabel("anim", true);
            }
        }
    } else if (revealingWhich == "done") {
        delayTimer = 0;
        revealingWhich = "none";
    } else if (revealingWhich != "none") {
        tallyScore();
    }
}

// AS: ScoreTallyScreen.checkAndIncrement
bool ScoreTallyScreen::checkAndIncrement()
{
    ++delayTimer;
    return delayTimer == delayBetweenReveals;
}

// AS: ScoreTallyScreen.scoreTallyClosed — the level is torn down, then the new customer's screen or the map
// (Continue), or the title (Quit).
void ScoreTallyScreen::scoreTallyClosed()
{
    if (endAction == "continue") {
        gameObj->obf_Ap();
        if (didUnlockNewCustomer) {
            gameObj->obf_4Z(unlockedNewCustomerIndex);
        } else {
            MainMenuParams params;
            params.section = "map";
            gameObj->showApiScreen("MainMenu", params);
        }
    } else if (endAction == "quit") {
        gameObj->obf_Ap();
        gameObj->showApiScreen("SplashScreen");
    }
    // api.§_-61§("EndOfDay") destroys this screen: showApiScreen above already has.
}

// AS: ScoreTallyScreen.clickContinue — the API's closeLeaderboard call is left out.
void ScoreTallyScreen::clickContinue()
{
    endAction = "continue";
    animatingOut = true;
    irisClip->gotoLabel("irisout", true);
    gameObj->menuBar.clear(); // api.§_-69§: the open API bars transition out
}

// AS: ScoreTallyScreen.clickQuit — the API's closeLeaderboard call is left out.
void ScoreTallyScreen::clickQuit()
{
    endAction = "quit";
    animatingOut = true;
    irisClip->gotoLabel("irisout", true);
    gameObj->menuBar.clear(); // api.§_-69§: the open API bars transition out
}

// AS: ScoreTallyScreen.setupCharacters — the player and the rescued customer jump in; three other unlocked
// customers stand in the crowd.
void ScoreTallyScreen::setupCharacters(bool setup)
{
    data::UserData &user = *gameObj->obf_51;
    engine::MovieClip *holder = clip->findChild("charholder");
    if (!holder) {
        return;
    }
    if (setup) {
        if (user.savedCharacterIndex > 0 && !user.savedCharacterWasUnlocked) {
            if (std::unique_ptr<engine::MovieClip> model = buildModel(user.savedCharacterIndex, 1)) {
                model->gotoAndStop(1);
                model->gotoLabel("jump", false);
                model->setPosition(static_cast<float>(unlockedStartX), static_cast<float>(unlockedStartY));
                unlockedChar = &holder->addChild(std::move(model), "");
            }
            didUnlockNewCustomer = true;
            unlockedNewCustomerIndex = user.savedCharacterIndex;
        }
        if (std::unique_ptr<engine::MovieClip> model = buildModel(user.selectedCharacter, user.selectedStyle)) {
            model->gotoAndStop(1);
            model->gotoLabel("jump", false);
            model->setScale(-0.5f, 0.5f);
            model->setPosition(static_cast<float>(playerStartX), static_cast<float>(playerStartY));
            playerChar = &holder->addChild(std::move(model), "");
        }
        std::vector<double> crowd;
        for (int i = 0; i < 28; ++i) {
            if (i < static_cast<int>(user.customersUnlocked.size()) && user.customersUnlocked[i] == 1 &&
                (i != user.savedCharacterIndex || user.savedCharacterWasUnlocked) && i != user.selectedCharacter) {
                crowd.push_back(i);
            }
        }
        for (int i = 1; i <= 3; ++i) {
            if (crowd.empty()) {
                continue;
            }
            const size_t pick = static_cast<size_t>(std::floor(as3::random() * static_cast<double>(crowd.size())));
            const double index = crowd[pick];
            crowd.erase(crowd.begin() + static_cast<std::ptrdiff_t>(pick));
            std::unique_ptr<engine::MovieClip> model = buildModel(index, 1);
            if (!model) {
                continue;
            }
            model->gotoAndStop(1);
            model->gotoLabel("standlobby", true);
            model->gotoAndPlay(model->currentFrame() + 1 + static_cast<int>(std::floor(as3::random() * 12)));
            model->setScale(-0.5f, 0.5f);
            if (i == 1) {
                model->setPosition(140, 200);
                crowdChar1 = &holder->addChild(std::move(model), "");
            } else if (i == 2) {
                model->setPosition(49, 198);
                crowdChar2 = &holder->addChild(std::move(model), "");
            } else {
                model->setPosition(91, 229);
                crowdChar3 = &holder->addChild(std::move(model), "");
            }
        }
    } else {
        for (engine::MovieClip **c : {&unlockedChar, &playerChar, &crowdChar1, &crowdChar2, &crowdChar3}) {
            if (*c) {
                holder->removeChild(*c);
                *c = nullptr;
            }
        }
    }
}

// AS: ScoreTallyScreen.updateCharacters — both jump down to their marks; when the player lands the crowd
// cheers (each with a different animation).
void ScoreTallyScreen::updateCharacters()
{
    double before = 0;
    if (playerJumping && playerChar) {
        before = playerJumpSpeed;
        playerJumpSpeed += 2;
        if (playerJumpSpeed > 20) {
            playerJumpSpeed = 20;
        }
        if (before < 0 && playerJumpSpeed >= 0) {
            playerChar->gotoLabel("fall", true);
        }
        double x = playerChar->x() + walkSpeed;
        if (x > playerTargetX) {
            x = playerTargetX;
        }
        double y = playerChar->y() + playerJumpSpeed;
        if (y > playerTargetY) {
            y = playerTargetY;
        }
        playerChar->setPosition(static_cast<float>(x), static_cast<float>(y));
        if (std::abs(playerTargetX - x) <= 1 && std::abs(playerTargetY - y) <= 1) {
            playerChar->setPosition(static_cast<float>(playerTargetX), static_cast<float>(playerTargetY));
            playerChar->gotoLabel("standhappy", true);
            playerJumping = false;
            playerLanded = true;
            std::vector<std::string> cheers = {"clap", "wave", "overjoyed", "standhappy", "standhappy"};
            for (engine::MovieClip *c : {crowdChar1, crowdChar2, crowdChar3}) {
                if (c) {
                    const size_t pick =
                        static_cast<size_t>(std::floor(as3::random() * static_cast<double>(cheers.size())));
                    c->gotoLabel(cheers[pick], true);
                    cheers.erase(cheers.begin() + static_cast<std::ptrdiff_t>(pick));
                }
            }
        }
    }
    if (unlockedChar && unlockedJumping) {
        before = unlockedJumpSpeed;
        unlockedJumpSpeed += 2;
        if (unlockedJumpSpeed > 20) {
            unlockedJumpSpeed = 20;
        }
        if (before < 0 && unlockedJumpSpeed >= 0) {
            unlockedChar->gotoLabel("fall", true);
        }
        double x = unlockedChar->x() - walkSpeed;
        if (x < unlockedTargetX) {
            x = unlockedTargetX;
        }
        double y = unlockedChar->y() + unlockedJumpSpeed;
        if (y > unlockedTargetY) {
            y = unlockedTargetY;
        }
        unlockedChar->setPosition(static_cast<float>(x), static_cast<float>(y));
        if (std::abs(unlockedTargetX - x) <= 1 && std::abs(unlockedTargetY - y) <= 1) {
            unlockedChar->setPosition(static_cast<float>(unlockedTargetX), static_cast<float>(unlockedTargetY));
            unlockedChar->gotoLabel("overjoyed", true);
            unlockedJumping = false;
            unlockedLanded = true;
        }
    }
}

// AS: ScoreTallyScreen.buildModel — a customer puppet (customerLobbyMC) dressed with the customer's parts and the
// player's weapon, at half size, taking no mouse events.
std::unique_ptr<engine::MovieClip> ScoreTallyScreen::buildModel(double whichCharacter, double style)
{
    const engine::SymbolLibrary &symbols = *gameObj->symbols;
    std::unique_ptr<engine::MovieClip> model = engine::MovieClip::create(symbols, "customerLobbyMC");
    if (!model) {
        return nullptr;
    }
    std::string name = gameObj->obf_3A->getCustomerClipName(whichCharacter);
    if (style == 2) {
        name += "2";
    } else if (style == 3) {
        name += "3";
    }
    auto attach = [&](const char *slot, const std::string &cls) {
        engine::MovieClip *holder = model->findChild(slot);
        std::unique_ptr<engine::MovieClip> part = engine::MovieClip::create(symbols, cls);
        if (holder && part) {
            holder->addChild(std::move(part), "clip");
        }
    };
    const std::string prefix = "customer_" + name + "_";
    attach("body", prefix + "body");
    attach("head", prefix + "head");
    attach("eyes", prefix + "eyes");
    attach("mouth", prefix + "mouth");
    attach("neck", prefix + "neck");
    attach("hair", prefix + "hair");
    attach("back_hair", prefix + "back_hair");
    attach("front_shoe", prefix + "foot");
    attach("back_shoe", prefix + "foot");
    attach("fronthand", prefix + "hand");
    attach("backhand", prefix + "hand2");
    attach("front_upperarm", prefix + "upperarm");
    attach("back_upperarm", prefix + "upperarm");
    attach("front_forearm", prefix + "forearm");
    attach("back_forearm", prefix + "forearm");
    attach("cross_backforearm", prefix + "forearm");
    attach("cross_back_hand", prefix + "hand2");
    // QUIRK: every customer on this screen holds the player's weapon.
    const std::string weapon = "weapon_" + gameObj->obf_3A->getWeaponClipName(gameObj->obf_51->selectedCharacter,
                                                                              gameObj->obf_51->selectedStyle);
    attach("weapon", weapon);
    attach("cross_back_weap", weapon);
    model->setScale(0.5f, 0.5f);
    model->mouseEnabled = false;
    model->mouseChildren = false;
    return model;
}

// AS: ScoreTallyScreen.destroy — the thumbnails are not disposed: they are cells of CustomerData's shared sheet.
void ScoreTallyScreen::destroy()
{
    setupCharacters(false);
    gameObj->removeContainerListener("clickContinue");
    gameObj->removeContainerListener("clickQuit");
    container->removeChild(irisClip);
    container->removeChild(clip);
    clip = nullptr;
    irisClip = nullptr;
}

} // namespace papaGame::screens
