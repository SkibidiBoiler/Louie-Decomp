#pragma once
// The port's menu bars: a top panel (title, tabs, a back button) and a bottom panel (the big action button), in
// place of the Flipline API's screen frame, which is not ported (it was part of the online API). Built from the
// SWF's own api_* panels and button pieces and laid out with the API's sizes; labels come from engine::Lang.
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "engine/movieclip.h"

namespace engine {

class Lang;

// The Flipline API's button (§_-7s§.§_-g§), rebuilt for the screens that place one themselves (the name entry's
// BACK / CONTINUE): an empty clip holding the left, middle and right pieces and the label. Its onClick is the
// API button's event. `width` > 0 fixes the width (the API's last argument).
std::unique_ptr<MovieClip> makeApiButton(const SymbolLibrary &symbols, TextLibrary *text, const std::string &label,
                                         bool large, float width = 0);

class MenuBar {
public:
    enum class Kind { Action, Tab, Back };
    struct Item {
        Kind kind = Kind::Tab;
        std::string labelKey; // engine::Lang key
        std::string event;    // the event name the screen listens for ("clickStart"...)
    };
    struct Layout {
        float topVisible = 43, bottomVisible = 101; // the API's default frame
        bool showTop = true, showBottom = true;
    };

    void setup(const SymbolLibrary &symbols, TextLibrary *text, const Lang *lang, const std::string &titleKey,
               const std::vector<Item> &items, const Layout &layout);
    void clear();
    bool active() const { return active_; }
    // api.disableButtons / enableButtons: the buttons stop taking the pointer (the bars still cover it).
    void setEnabled(bool enabled) { enabled_ = enabled; }
    // api.§_-AG§(title): a new title for the current screen.
    void setTitle(const Lang *lang, const std::string &titleKey);
    // Port: hide the bottom panel while it holds no button (the credits picture reaches the bottom of the stage).
    void setBottomHidden(bool hidden);
    // The pointer for one logic frame: calls `onEvent(event)` when a button is clicked. True when the pointer
    // is over the bars (the screen below should not get it).
    bool update(const PointerState &pointer, const std::function<void(const std::string &)> &onEvent);
    void render(const RenderContext &ctx) const;

private:
    struct Button {
        Item item;
        std::string label;
        float x = 0, y = 0, w = 0, h = 0, piece = 16;
        bool large = false, red = false, over = false, down = false;
        bool tight = false; // no side padding (a row of tabs too long for the bar)
        std::vector<std::unique_ptr<MovieClip>> pieces, overlays;
    };
    void buildButton(Button &b);

    const SymbolLibrary *symbols_ = nullptr;
    TextLibrary *text_ = nullptr;
    bool active_ = false;
    bool enabled_ = true;
    std::string title_;
    Layout layout_;
    std::unique_ptr<MovieClip> top_, bottom_;
    bool bottomHidden_ = false;
    float topY_ = 0, bottomY_ = 0;
    std::vector<Button> buttons_;
    int pressed_ = -1;
};

} // namespace engine
