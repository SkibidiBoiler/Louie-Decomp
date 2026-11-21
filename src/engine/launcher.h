#pragma once
// The port's game picker (not in any original game), the first screen, laid out like a console's home screen: a
// row of boxes with each game's cover, the selected one larger, and the screen behind them dressed in that game's
// art and colours (App plays its title music). A game with imported data and a port starts; one without data
// offers to import the owner's .swf (through ImportScreen); one not ported yet is shown as coming soon. The
// covers are drawn from the imported game's own title screen at run time (App::buildCovers), never stored.
// Drawn in window pixels; works with the mouse, the keyboard, a gamepad (as keys, see App) and touch.
#include <SDL3/SDL.h>
#include <string>

namespace engine {

class ImportScreen;
class Lang;
class TextLibrary;

class Launcher {
public:
    Launcher(Lang &lang, ImportScreen &import, std::string dataDir);
    ~Launcher();
    Launcher(const Launcher &) = delete;
    Launcher &operator=(const Launcher &) = delete;

    // Checks again which games have imported data (after an import).
    void refresh();
    void select(const std::string &gameId);
    const char *selectedGame() const { return games_[selected_].id; }
    bool imported(const std::string &gameId) const;

    // The game's title art: the stage strip y 72..296 (700 x 224) of its title screen. The picker owns it.
    void setCover(const std::string &gameId, SDL_Texture *banner);
    bool hasCover(const std::string &gameId) const;
    void clearCovers();

    // Animations (seconds since the last call).
    void update(float dt);

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
        bool ported;      // SKELETON: only PL2 is playable so far
        SDL_FColor theme; // the box and background colours when there is no cover
        bool imported = false;
        SDL_Texture *cover = nullptr;
    };
    static constexpr int GameCount = 3;
    // What the keyboard / gamepad acts on: the selected game's box and action button, or the quit button.
    enum class Focus { Games, Quit };
    // Where something is: a game box, the selected game's action button, the quit button.
    enum class Target { None, Box, Action, Quit };
    struct Layout {
        float pad = 0;
        SDL_FRect boxes[GameCount]{};
        float infoY = 0, titleSize = 0;
        SDL_FRect action{}, quit{};
    };
    Layout layout(int w, int h) const;
    Target hit(float x, float y, int w, int h, int &box) const;
    void activate();
    void move(int delta);
    std::string statusText(const Game &g) const;
    std::string actionText(const Game &g) const;
    void renderBackground(SDL_Renderer *r, int w, int h) const;
    void renderBox(SDL_Renderer *r, TextLibrary &text, int i, const SDL_FRect &box) const;

    Lang &lang_;
    ImportScreen &import_;
    std::string dataDir_;
    // The port's order. Theme colours: PL2's lettuce green, PL3's sundae pink, PL1's pizza red.
    Game games_[GameCount] = {{"pl2", true, {0.16f, 0.42f, 0.22f, 1}},
                              {"pl3", false, {0.55f, 0.22f, 0.45f, 1}},
                              {"pl1", false, {0.62f, 0.2f, 0.12f, 1}}};
    int selected_ = 0;
    int previous_ = 0;      // the game the background fades from
    float fade_ = 1;        // 0..1: background cross-fade from previous_ to selected_
    float scale_[GameCount] = {1, 0.78f, 0.78f}; // box sizes (animated)
    Focus focus_ = Focus::Games;
    Target pressed_ = Target::None;
    int pressedBox_ = -1;
    bool quit_ = false;
    std::string chosen_;
};

} // namespace engine
