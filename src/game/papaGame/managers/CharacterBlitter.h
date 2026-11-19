#pragma once
// AS: papaGame.managers.CharacterBlitter — dresses the customer puppet and draws its animation frames into the
// torso / front foot / back foot sheets the game blits from. The port draws into render targets.
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "engine/movieclip.h"
#include "engine/sheets.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::managers {

class CharacterBlitter {
public:
    explicit CharacterBlitter(obf_9G::obf_4R *gameObj);
    ~CharacterBlitter();
    CharacterBlitter(const CharacterBlitter &) = delete;
    CharacterBlitter &operator=(const CharacterBlitter &) = delete;

    // The original spreads the work over frames at 200 fps and calls back; the port finishes before returning.
    void createPlayerSprite(const std::function<void()> &callback = {}, bool isCaged = false,
                            const std::string &cagedName = {});
    void destroy();

    engine::Sheet torsoSheet;
    double spritesPerRow = 1;
    double spriteWidth = 1;
    double spriteHeight = 1;
    double flipBlitOffset = 1;
    double spriteCenterX = 1;
    double spriteCenterY = 1;
    double spriteTargetX = 1;
    double spriteTargetY = 1;
    double blitOffsetX = 0;
    double blitFlippedOffsetX = 0;
    double torsoOffsetX = 0;
    double torsoOffsetFlippedX = 0;
    double torsoOffsetY = 0;
    engine::Sheet frontFootSheet;
    engine::Sheet backFootSheet;
    double footSpritesPerRow = 1;
    double footSpriteWidth = 1;
    double footSpriteHeight = 1;
    double footFlipBlitOffset = 1;
    std::string whichCharacter = "Marty";
    double whichStyle = 1;
    bool isCaged = false;
    obf_9G::obf_4R *gameObj = nullptr;

private:
    void setupFramesNeeded();
    void startBlittingPhase();
    void blitFrame();
    void finishBlittingPhase();
    void finishBlitting();
    void setupModelForPhase();
    void buildModel();
    void cleanupModel();
    engine::Sheet createSheet(double width, double height);
    void drawModel(engine::Sheet &sheet, engine::MovieClip &model, const engine::Affine &matrix);

    std::vector<std::unique_ptr<engine::MovieClip>> models;
    double numModels = 5;
    double modelScale = 0.27;
    double maxSheetPxWidth = 8191;
    std::string whichBlitPhase = "torso";
    double currentAnimFrame = 1;
    double startAnimFrame = 1;
    double totalAnimFrames = 0;
    std::vector<double> framesNeeded;
    double boundsWidth = 0;
    double boundsHeight = 0;
    double boundsX = 0;
    double boundsY = 0;
    engine::Affine blitMatrix;
    engine::Affine blitMatrixFlipped;
    double blitSpritesPerRow = 0;
    std::function<void()> callback;
    bool blitFlipped = true;
    bool finished_ = false; // port: ends the synchronous loop
};

} // namespace papaGame::managers
