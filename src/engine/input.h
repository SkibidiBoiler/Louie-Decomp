#pragma once
// Input by actions: keyboard, gamepad and on-screen touch buttons all feed the same six actions the game
// reads (GameControls.keyPressedLeft/Right/Up/Down/Jump/Action).
#include <SDL3/SDL.h>
#include <array>
#include <unordered_map>

namespace engine {

enum class Action { Left, Right, Up, Down, Jump, Attack, Count };

struct ActionState {
    std::array<bool, static_cast<size_t>(Action::Count)> held{};
    bool operator[](Action a) const { return held[static_cast<size_t>(a)]; }
};

// On-screen buttons for touch screens: translucent circles anchored to the window corners.
class TouchControls {
public:
    // Handles finger events; returns true when the event was used by a button.
    bool event(const SDL_Event &ev, SDL_Window *window);
    // Draws in window pixels (call with the renderer's logical presentation disabled).
    void render(SDL_Renderer *renderer, int windowW, int windowH) const;
    void held(ActionState &out) const;

    bool visible = false;   // shown after the first touch (always on Android)
    float opacity = 0.55f;  // overall transparency, 0..1
    float size = 1.0f;      // button size multiplier

private:
    struct Button {
        Action action;
        float cx, cy;   // centre: x from the left (negative = from the right), y from the bottom, in button units
        float radius;   // in button units
    };
    void layout(int windowW, int windowH, const Button &b, float &px, float &py, float &pr) const;
    int hit(float x, float y, int windowW, int windowH) const;

    static const Button kButtons[5];
    std::unordered_map<SDL_FingerID, int> fingers_; // finger -> button index
};

// Flash keyboard key codes (KeyboardEvent.keyCode, what the game saves) <-> SDL scancodes; 0 / UNKNOWN when none.
SDL_Scancode flashKeyToScancode(int keyCode);
int scancodeToFlashKey(SDL_Scancode sc);

class Input {
public:
    void event(const SDL_Event &ev, SDL_Window *window);
    // Current actions from every source.
    ActionState poll() const;

    // The game's keys (Flash key codes) for Left, Right, Up, Down, Jump, Attack: the original's defaults until the
    // game sets the player's own (its CONTROLS tab).
    std::array<int, 6> keyBindings{37, 39, 38, 40, 38, 32};

    TouchControls touch;

private:
    SDL_Gamepad *gamepad_ = nullptr;
};

} // namespace engine
