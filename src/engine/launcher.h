#pragma once
// The port's game picker (not in any original game): the first screen, a translucent panel like the options
// menu's listing the games the port knows. A game with imported data and a port starts; one without data offers
// to import the owner's .swf (through ImportScreen); one not ported yet is shown as coming soon. Drawn in window
// pixels; works with the mouse, the keyboard, a gamepad (as keys, see App) and touch.
#include <SDL3/SDL.h>
#include <string>

namespace engine {

class ImportScreen;
class Lang;
class TextLibrary;

class Launcher {
public:
    Launcher(Lang &lang, ImportScreen &import, std::string dataDir);

    // Checks again which games have imported data (after an import).
    void refresh();
    void select(const std::string &gameId);

    // Pointer in window pixels of a w x h output. The picker owns the whole screen.
    void pointerMove(float x, float y, int w, int h);
    void pointerDown(float x, float y, int w, int h);
    void pointerUp(float x, float y, int w, int h);
    // A key (gamepad buttons arrive as keys); true when used. Escape is left to the app (it quits).
    bool key(SDL_Keycode key);

    // The game the player chose to play since the last call ("" if none).
    std::string takeChosen();
    bool quitRequested() const { return quit_; }

    void render(SDL_Renderer *renderer, TextLibrary &text, int w, int h) const;

private:
    struct Game {
        const char *id;
        bool ported; // SKELETON: only PL2 is playable so far
        bool imported = false;
    };
    static constexpr int GameCount = 3, QuitRow = GameCount, RowCount = GameCount + 1;
    struct Layout {
        float panelX = 0, panelY = 0, panelW = 0, panelH = 0;
        float titleH = 0, rowH = 0, quitH = 0, statusH = 0;
    };
    Layout layout(int w, int h) const;
    int rowAt(float x, float y, const Layout &l) const;
    void activate(int row);
    std::string statusText(const Game &g) const;
    std::string actionText(const Game &g) const;

    Lang &lang_;
    ImportScreen &import_;
    std::string dataDir_;
    Game games_[GameCount] = {{"pl2", true}, {"pl3", false}, {"pl1", false}}; // the port's order
    int selected_ = 0;
    int pressed_ = -1;
    bool quit_ = false;
    std::string chosen_;
};

} // namespace engine
