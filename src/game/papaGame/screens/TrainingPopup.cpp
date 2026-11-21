#include "game/papaGame/screens/TrainingPopup.h"

#include <cctype>
#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::screens {

// AS: TrainingPopup.TrainingPopup
TrainingPopup::TrainingPopup(obf_9G::obf_4R *gameObj_, std::string trainingFlag_, bool isInGame_, std::string key1_,
                             std::string key2_, std::string key3_, double timeout_)
    : gameObj(gameObj_), trainingFlag(std::move(trainingFlag_)), isInGame(isInGame_), key1(std::move(key1_)),
      key2(std::move(key2_)), key3(std::move(key3_)), timeout(timeout_)
{
    setupPopup();
}

// Port: the three copies of the key-cap set-up in setupPopup. A TextField's `visible = false` is drawn as an empty
// text (the port cannot hide a timeline text field).
void TrainingPopup::setupKey(const char *instance, const std::string &key, const std::string &wordText)
{
    engine::MovieClip *cap = clip->findChild(instance);
    if (!cap) {
        return;
    }
    if (key.empty()) {
        cap->setVisible(false);
        return;
    }
    cap->setVisible(true);
    if (key == "Left" || key == "Right" || key == "Up" || key == "Down") {
        std::string lower = key;
        for (char &ch : lower) {
            ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        }
        cap->gotoLabel(lower, false);
        cap->setText("letter_txt", "");
        cap->setText("word_txt", "");
    } else if (key.size() > 1) {
        cap->gotoLabel("wide", false);
        cap->setText("letter_txt", "");
        cap->setText("word_txt", wordText);
    } else {
        cap->gotoLabel("other", false);
        cap->setText("letter_txt", key);
        cap->setText("word_txt", "");
    }
}

// AS: TrainingPopup.setupPopup — QUIRK: a wide third key shows the first key's name.
void TrainingPopup::setupPopup()
{
    engine::MovieClip &container = isInGame ? *gameObj->obf_5h : *gameObj->obf_9X;
    clip = &container.addChild(engine::MovieClip::create(*gameObj->symbols, "trainingMC"), "");
    clip->gotoLabel(trainingFlag, false);
    setupKey("key1", key1, key1);
    setupKey("key2", key2, key2);
    setupKey("key3", key3, key1);
    if (isInGame) {
        isTweeningIn = true;
        clip->setY(static_cast<float>(offscreenY));
    } else {
        isTweeningIn = false;
        clip->setY(0);
    }
    clip->mouseEnabled = false;
    clip->mouseChildren = false;
}

// AS: TrainingPopup.updatePopup (ENTER_FRAME) — slide in, count down the timeout, slide out.
void TrainingPopup::updatePopup()
{
    if (!clip) {
        return;
    }
    const double y = clip->y();
    if (isTweeningIn) {
        const double distance = 0 - y;
        clip->setY(static_cast<float>(y + distance / 4));
        if (std::abs(distance) <= 1) {
            clip->setY(0);
            isTweeningIn = false;
        }
    } else if (isTweeningOut) {
        const double distance = offscreenY - y;
        clip->setY(static_cast<float>(y + distance / 4));
        if (std::abs(distance) <= 1) {
            clip->setY(static_cast<float>(offscreenY));
            isTweeningOut = false;
        }
    } else if (timeout != -1) {
        ++animTimer;
        if (animTimer == timeout) {
            closePopup();
        }
    }
}

// AS: TrainingPopup.closePopup
void TrainingPopup::closePopup(const std::string *flag)
{
    if (!flag || *flag == trainingFlag) {
        isTweeningOut = true;
        isTweeningIn = false;
    }
}

// AS: TrainingPopup.removePopup
void TrainingPopup::removePopup()
{
    gameObj->obf_64(false);
}

// AS: TrainingPopup.destroy
void TrainingPopup::destroy()
{
    if (clip) {
        if (gameObj->obf_5h) {
            gameObj->obf_5h->removeChild(clip);
        }
        if (gameObj->obf_9X) {
            gameObj->obf_9X->removeChild(clip);
        }
        clip = nullptr;
    }
}

} // namespace papaGame::screens
