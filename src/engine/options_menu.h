#pragma once
// The port's options menu (not in the original): a translucent panel over the game, opened from a gear button in
// the window's top-right corner (or F1). It edits engine::Settings and saves them at once. Drawn in window pixels
// in the same style as the on-screen touch buttons. The game is paused while it is open.
#include <SDL3/SDL.h>
#include <string>

namespace engine {

class Lang;
class TextLibrary;
class TouchControls;
struct Settings;

class OptionsMenu {
public:
    OptionsMenu(Settings &settings, Lang &lang, TouchControls &touch) : settings_(settings), lang_(lang), touch_(touch) {}

    bool isOpen() const { return open_; }
    void setOpen(bool open);
    void setOriginalRate(int rate) { originalRate_ = rate; }
    // Puts the saved touch settings on the buttons (call after Settings::load).
    void applySettings();

    // Pointer press / release in window pixels. True when the menu used it (the game must not get it).
    bool pointerDown(float x, float y, int w, int h);
    bool pointerUp(float x, float y, int w, int h);
    // A key; true when used.
    bool key(SDL_Keycode key);

    void render(SDL_Renderer *renderer, TextLibrary &text, int w, int h) const;

private:
    enum Row { Fps, Interpolation, Language, TouchOpacity, TouchSize, Overlay, Close, RowCount };
    struct Layout {
        float gearX = 0, gearY = 0, gearR = 0;
        float panelX = 0, panelY = 0, panelW = 0, rowH = 0, titleH = 0;
    };
    Layout layout(int w, int h) const;
    int rowAt(float x, float y, const Layout &l) const;
    void change(int row, int dir);
    const char *labelKey(int row) const;
    std::string valueText(int row) const;

    Settings &settings_;
    Lang &lang_;
    TouchControls &touch_;
    int originalRate_ = 30;
    bool open_ = false;
    bool gearPressed_ = false;
    int selected_ = 0;
};

} // namespace engine
