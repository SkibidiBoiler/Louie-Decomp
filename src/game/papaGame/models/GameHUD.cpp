#include "game/papaGame/models/GameHUD.h"

#include "game/obf_9G/obf_4R.h"
#include "game/obf_AF/obf_81.h"
#include "game/papaGame/screens/MainMenuScreen.h"

using obf_AF::obf_81;

namespace papaGame::models {

// AS: GameHUD constructor
GameHUD::GameHUD(obf_9G::obf_4R *gameObj_) : gameObj(gameObj_)
{
    setupHUD();
}

// AS: GameHUD.setupHUD (SKELETON: the FPS toggle, the portrait and the bubbles)
void GameHUD::setupHUD()
{
    const engine::SymbolLibrary &symbols = *gameObj->symbols;
    for (const auto &[name, id] : symbols.classes()) {
        if (name == "hudMC") {
            if (const engine::Timeline *tl = symbols.timeline(id)) {
                interfaceClip = std::make_unique<engine::MovieClip>(symbols, *tl);
            }
        }
    }
    if (!interfaceClip) {
        return;
    }
    interfaceClip->mouseEnabled = false;
    updateDisplay();
    updateTimer();
    if (engine::MovieClip *menu = interfaceClip->findChild("menu_btn")) {
        menu->onClick = [this] { clickMenu(); };
    }
    if (engine::MovieClip *mute = interfaceClip->findChild("mute_btn")) {
        mute->onClick = [this] { clickMute(); };
    }
    if (engine::MovieClip *unmute = interfaceClip->findChild("unmute_btn")) {
        unmute->onClick = [this] { clickUnmute(); };
    }
    interfaceClip->setChildAlpha("fps_txt", 0);
    updateMuteButton();
    if (const data::CustomerDataFile *c = gameObj->obf_3A->getCustomerData(gameObj->obf_51->selectedCharacter)) {
        if (engine::MovieClip *skill = interfaceClip->findChild("skill")) {
            skill->gotoLabel(c->skillType, false);
        }
    }
    hideHUD();
}

// AS: GameHUD.updateDisplay
void GameHUD::updateDisplay()
{
    if (!interfaceClip) {
        return;
    }
    const data::UserData &user = *gameObj->obf_51;
    const double level = gameObj->obf_AQ->currentLevel;
    const managers::ChallengeManager &challenges = *gameObj->obf_94;
    interfaceClip->setText("points_txt", "<b>" + obf_81::obf_7w(user.points) + " PTS</b>");
    if (level < 8) {
        interfaceClip->setText("coins_txt", "<b>" + obf_81::obf_7w(user.money) + "/" +
                                                challenges.getChallengeTargetAmount(level, 6) + "</b>");
        interfaceClip->setText("burgers_txt", "<b>" + as3::String(user.burgerzillas) + "/" +
                                                  challenges.getChallengeTargetAmount(level, 5) + "</b>");
        interfaceClip->setText("special_txt", "<b>" + as3::String(user.specialitems) + "/" +
                                                  challenges.getChallengeTargetAmount(level, 4) + "</b>");
    } else {
        interfaceClip->setText("coins_txt", "<b>" + obf_81::obf_7w(user.money) + "</b>");
        interfaceClip->setText("burgers_txt", "");
        interfaceClip->setText("special_txt", "");
        if (engine::MovieClip *icon = interfaceClip->findChild("burgericon")) {
            icon->setVisible(false);
        }
        if (engine::MovieClip *icon = interfaceClip->findChild("specialicon")) {
            icon->setVisible(false);
        }
    }
    if (engine::MovieClip *icon = interfaceClip->findChild("specialicon")) {
        icon->gotoAndStop(static_cast<int>(level) + 1);
    }
}

// AS: GameHUD.updateTimer
void GameHUD::updateTimer()
{
    if (interfaceClip) {
        const double frames = gameObj->obf_8o ? gameObj->obf_8o->gameplayTimer : 0;
        interfaceClip->setText("timer_txt", obf_81::obf_2i(frames, true));
    }
}

// AS: GameHUD.hideHUD
void GameHUD::hideHUD()
{
    if (interfaceClip) {
        interfaceClip->setVisible(false);
    }
}

// AS: GameHUD.showHUD (updateAdjustmentNumbers is empty)
void GameHUD::showHUD()
{
    if (interfaceClip) {
        interfaceClip->setVisible(true);
    }
}

// AS: GameHUD.clickMenu — the game pauses and the pause menu opens on the area's info.
void GameHUD::clickMenu()
{
    if (!gameObj->obf_71->isTransitioningIn && !gameObj->obf_71->isTransitionOut) {
        gameObj->obf_8o->pauseGame();
        papaGame::screens::MainMenuParams params;
        params.section = "info";
        gameObj->showApiScreen("PauseMenu", params);
    }
}

// AS: GameHUD.clickMute
void GameHUD::clickMute()
{
    gameObj->obf_7C->muteSound(true);
    updateMuteButton();
}

// AS: GameHUD.clickUnmute
void GameHUD::clickUnmute()
{
    gameObj->obf_7C->unmuteSound(true);
    updateMuteButton();
}

// AS: GameHUD.updateMuteButton
void GameHUD::updateMuteButton()
{
    if (interfaceClip) {
        if (engine::MovieClip *mute = interfaceClip->findChild("mute_btn")) {
            mute->setVisible(!gameObj->obf_7C->isMute);
        }
        if (engine::MovieClip *unmute = interfaceClip->findChild("unmute_btn")) {
            unmute->setVisible(gameObj->obf_7C->isMute);
        }
    }
}

// AS: GameHUD.updatePlayerHealth — one heart per point of health.
void GameHUD::updatePlayerHealth(double health)
{
    if (!interfaceClip) {
        return;
    }
    for (int i = 1; i <= 3; ++i) {
        if (engine::MovieClip *heart = interfaceClip->findChild("heart" + std::to_string(i))) {
            heart->setVisible(health >= i);
        }
    }
}

void GameHUD::render(const engine::RenderContext &ctx) const
{
    if (interfaceClip && interfaceClip->visible()) {
        interfaceClip->render(ctx, engine::Affine{}, engine::ColorTransform{});
    }
}

} // namespace papaGame::models
