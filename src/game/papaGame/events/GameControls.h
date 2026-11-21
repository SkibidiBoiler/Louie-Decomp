#pragma once
// AS: papaGame.events.GameControls — reads the keys and drives the player every frame (detectKeys).
// The port fills keyPressed* from engine::Input actions (keyboard, gamepad, touch) instead of key events.
// Ported so far: the player part of detectKeys, keyCheckPlayer (walking, turning, ducking, air control) and
// jumpPlayer (jump, short jump, coyote time). SKELETON: ladders, attacks, carried objects, skills
// (pound, double jump, glide, wall jump, crawl, push, slide), skidding, being shoved/hit, doors, pause.

namespace obf_9G { class obf_4R; }
namespace papaGame::models::characters { class CustomerChar; }

namespace papaGame::events {

class GameControls {
public:
    explicit GameControls(obf_9G::obf_4R *gameObj_) : gameObj(gameObj_) {}

    // One logic frame for the player (the player part of detectKeys).
    void detectKeys();
    void keyCheckPlayer(models::characters::CustomerChar &p);
    void jumpPlayer(models::characters::CustomerChar &p);
    void resetKeys();
    void jumpPlayerOnLadder(); // AS: GameControls.jumpPlayerOnLadder
    void pauseGame();
    void resumeGame(bool keysWereChanged = false);
    void travelThroughDoor(double roomID, double doorID, double xOffset, double yOffset, double dirX, double dirY);

    obf_9G::obf_4R *gameObj = nullptr;
    bool keyPressedLeft = false, keyPressedRight = false, keyPressedUp = false, keyPressedDown = false;
    bool keyPressedJump = false, keyPressedAction = false;
    double gameplayTimer = 0;
    double currentLevelTimer = 0;
    bool isPaused = false, stopCycle = false, stopControls = false;
    bool stopActionControls = false, stopHurtControls = false;
    bool travelingThroughDoor = false;
    double doorRoomID = 0, doorDoorID = 0, doorXoffset = 0, doorYoffset = 0, doorDirX = 0, doorDirY = 0;
};

} // namespace papaGame::events
