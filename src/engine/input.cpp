#include "engine/input.h"

#include <algorithm>
#include <cmath>

namespace engine {

namespace {

void set(ActionState &s, Action a) { s.held[static_cast<size_t>(a)] = true; }

// Filled polygon (triangle fan around the first point) in one colour.
void fan(SDL_Renderer *r, const SDL_FPoint *pts, int n, SDL_FColor c)
{
    if (n < 3) {
        return;
    }
    SDL_Vertex v[64];
    int idx[186];
    int vi = 0, ii = 0;
    for (int i = 0; i < n && i < 64; ++i) {
        v[vi++] = SDL_Vertex{pts[i], c, {0, 0}};
    }
    for (int i = 1; i + 1 < vi; ++i) {
        idx[ii++] = 0;
        idx[ii++] = i;
        idx[ii++] = i + 1;
    }
    SDL_RenderGeometry(r, nullptr, v, vi, idx, ii);
}

void disc(SDL_Renderer *r, float cx, float cy, float radius, SDL_FColor c)
{
    SDL_FPoint pts[49];
    pts[0] = {cx, cy};
    for (int i = 0; i <= 47; ++i) {
        const float a = static_cast<float>(i) / 47.0f * 6.2831853f;
        pts[i + 1] = {cx + std::cos(a) * radius, cy + std::sin(a) * radius};
    }
    fan(r, pts, 49, c);
}

void ring(SDL_Renderer *r, float cx, float cy, float radius, float thickness, SDL_FColor c)
{
    constexpr int kSeg = 48;
    SDL_Vertex v[2 * (kSeg + 1)];
    int idx[6 * kSeg];
    for (int i = 0; i <= kSeg; ++i) {
        const float a = static_cast<float>(i) / kSeg * 6.2831853f;
        const float ca = std::cos(a), sa = std::sin(a);
        v[2 * i] = SDL_Vertex{{cx + ca * radius, cy + sa * radius}, c, {0, 0}};
        v[2 * i + 1] = SDL_Vertex{{cx + ca * (radius - thickness), cy + sa * (radius - thickness)}, c, {0, 0}};
    }
    for (int i = 0; i < kSeg; ++i) {
        const int a = 2 * i;
        const int k = 6 * i;
        idx[k] = a;
        idx[k + 1] = a + 1;
        idx[k + 2] = a + 2;
        idx[k + 3] = a + 1;
        idx[k + 4] = a + 3;
        idx[k + 5] = a + 2;
    }
    SDL_RenderGeometry(r, nullptr, v, 2 * (kSeg + 1), idx, 6 * kSeg);
}

// Rounded-looking arrow head pointing at `angle` (radians), centred on (cx, cy).
void arrow(SDL_Renderer *r, float cx, float cy, float s, float angle, SDL_FColor c)
{
    const float ca = std::cos(angle), sa = std::sin(angle);
    auto rot = [&](float x, float y) { return SDL_FPoint{cx + x * ca - y * sa, cy + x * sa + y * ca}; };
    const SDL_FPoint tri[3] = {rot(s * 0.55f, 0), rot(-s * 0.35f, -s * 0.5f), rot(-s * 0.35f, s * 0.5f)};
    fan(r, tri, 3, c);
}

} // namespace

// x: from the left edge (negative: from the right edge); y: up from the bottom edge. Units: button unit.
const TouchControls::Button TouchControls::kButtons[5] = {
    {Action::Left, 1.25f, 1.35f, 0.95f},
    {Action::Right, 3.45f, 1.35f, 0.95f},
    {Action::Down, 2.35f, 3.15f, 0.7f},
    {Action::Jump, -1.35f, 1.55f, 1.05f},
    {Action::Attack, -3.55f, 1.15f, 0.8f},
};

void TouchControls::layout(int w, int h, const Button &b, float &px, float &py, float &pr) const
{
    const float unit = static_cast<float>(std::min(w, h)) / 9.0f * size;
    px = b.cx >= 0 ? b.cx * unit : static_cast<float>(w) + b.cx * unit;
    py = static_cast<float>(h) - b.cy * unit;
    pr = b.radius * unit;
}

int TouchControls::hit(float x, float y, int w, int h) const
{
    int best = -1;
    float bestDist = 1e30f;
    for (int i = 0; i < 5; ++i) {
        float px, py, pr;
        layout(w, h, kButtons[i], px, py, pr);
        const float d = std::hypot(x - px, y - py);
        if (d < pr * 1.25f && d < bestDist) { // a little forgiving around each button
            best = i;
            bestDist = d;
        }
    }
    return best;
}

bool TouchControls::event(const SDL_Event &ev, SDL_Window *window)
{
    if (ev.type != SDL_EVENT_FINGER_DOWN && ev.type != SDL_EVENT_FINGER_MOTION && ev.type != SDL_EVENT_FINGER_UP &&
        ev.type != SDL_EVENT_FINGER_CANCELED) {
        return false;
    }
    visible = true;
    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(window, &w, &h);
    const float x = ev.tfinger.x * static_cast<float>(w), y = ev.tfinger.y * static_cast<float>(h);
    const SDL_FingerID id = ev.tfinger.fingerID;
    if (ev.type == SDL_EVENT_FINGER_UP || ev.type == SDL_EVENT_FINGER_CANCELED) {
        return fingers_.erase(id) > 0;
    }
    const int b = hit(x, y, w, h);
    if (ev.type == SDL_EVENT_FINGER_DOWN) {
        if (b < 0) {
            return false;
        }
        fingers_[id] = b;
        return true;
    }
    // Motion: a finger held on a button can slide to a neighbour (e.g. from left to right).
    auto it = fingers_.find(id);
    if (it == fingers_.end()) {
        return false;
    }
    if (b >= 0) {
        it->second = b;
    }
    return true;
}

void TouchControls::held(ActionState &out) const
{
    for (const auto &f : fingers_) {
        const Action a = kButtons[f.second].action;
        set(out, a);
        if (a == Action::Jump) {
            set(out, Action::Up); // the original's jump key is also its up key
        }
    }
}

void TouchControls::render(SDL_Renderer *r, int w, int h) const
{
    if (!visible) {
        return;
    }
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    for (int i = 0; i < 5; ++i) {
        const Button &b = kButtons[i];
        bool pressed = false;
        for (const auto &f : fingers_) {
            pressed = pressed || f.second == i;
        }
        float px, py, pr;
        layout(w, h, b, px, py, pr);
        const float o = opacity;
        disc(r, px, py, pr, SDL_FColor{0.05f, 0.07f, 0.12f, (pressed ? 0.45f : 0.22f) * o});
        ring(r, px, py, pr, std::max(2.0f, pr * 0.06f), SDL_FColor{1, 1, 1, (pressed ? 0.95f : 0.6f) * o});
        const SDL_FColor icon{1, 1, 1, (pressed ? 1.0f : 0.75f) * o};
        const float s = pr * 0.62f;
        switch (b.action) {
        case Action::Left: arrow(r, px - s * 0.08f, py, s, 3.14159265f, icon); break;
        case Action::Right: arrow(r, px + s * 0.08f, py, s, 0, icon); break;
        case Action::Down: arrow(r, px, py + s * 0.08f, s, 1.5707963f, icon); break;
        case Action::Jump: arrow(r, px, py - s * 0.08f, s, -1.5707963f, icon); break;
        case Action::Attack: {
            // A simple four-point spark.
            const SDL_FPoint star[9] = {{px, py},
                                        {px, py - s * 0.62f},
                                        {px + s * 0.16f, py - s * 0.16f},
                                        {px + s * 0.62f, py},
                                        {px + s * 0.16f, py + s * 0.16f},
                                        {px, py + s * 0.62f},
                                        {px - s * 0.16f, py + s * 0.16f},
                                        {px - s * 0.62f, py},
                                        {px - s * 0.16f, py - s * 0.16f}};
            SDL_FPoint closed[10];
            std::copy(star, star + 9, closed);
            closed[9] = star[1];
            fan(r, closed, 10, icon);
            break;
        }
        default: break;
        }
    }
}

void Input::event(const SDL_Event &ev, SDL_Window *window)
{
    if (ev.type == SDL_EVENT_GAMEPAD_ADDED && !gamepad_) {
        gamepad_ = SDL_OpenGamepad(ev.gdevice.which);
    } else if (ev.type == SDL_EVENT_GAMEPAD_REMOVED && gamepad_ && SDL_GetGamepadID(gamepad_) == ev.gdevice.which) {
        SDL_CloseGamepad(gamepad_);
        gamepad_ = nullptr;
    }
    (void)window; // touch buttons get their events from the caller (only while playing)
}

SDL_Scancode flashKeyToScancode(int c)
{
    if (c >= 65 && c <= 90) {
        return static_cast<SDL_Scancode>(SDL_SCANCODE_A + (c - 65));
    }
    if (c >= 49 && c <= 57) {
        return static_cast<SDL_Scancode>(SDL_SCANCODE_1 + (c - 49));
    }
    if (c >= 97 && c <= 105) {
        return static_cast<SDL_Scancode>(SDL_SCANCODE_KP_1 + (c - 97));
    }
    if (c >= 112 && c <= 123) {
        return static_cast<SDL_Scancode>(SDL_SCANCODE_F1 + (c - 112));
    }
    switch (c) {
    case 48: return SDL_SCANCODE_0;
    case 96: return SDL_SCANCODE_KP_0;
    case 8: return SDL_SCANCODE_BACKSPACE;
    case 9: return SDL_SCANCODE_TAB;
    case 13: return SDL_SCANCODE_RETURN;
    case 16: return SDL_SCANCODE_LSHIFT;
    case 17: return SDL_SCANCODE_LCTRL;
    case 18: return SDL_SCANCODE_LALT;
    case 20: return SDL_SCANCODE_CAPSLOCK;
    case 32: return SDL_SCANCODE_SPACE;
    case 33: return SDL_SCANCODE_PAGEUP;
    case 34: return SDL_SCANCODE_PAGEDOWN;
    case 35: return SDL_SCANCODE_END;
    case 36: return SDL_SCANCODE_HOME;
    case 37: return SDL_SCANCODE_LEFT;
    case 38: return SDL_SCANCODE_UP;
    case 39: return SDL_SCANCODE_RIGHT;
    case 40: return SDL_SCANCODE_DOWN;
    case 45: return SDL_SCANCODE_INSERT;
    case 46: return SDL_SCANCODE_DELETE;
    case 106: return SDL_SCANCODE_KP_MULTIPLY;
    case 107: return SDL_SCANCODE_KP_PLUS;
    case 109: return SDL_SCANCODE_KP_MINUS;
    case 110: return SDL_SCANCODE_KP_PERIOD;
    case 111: return SDL_SCANCODE_KP_DIVIDE;
    case 186: return SDL_SCANCODE_SEMICOLON;
    case 187: return SDL_SCANCODE_EQUALS;
    case 188: return SDL_SCANCODE_COMMA;
    case 189: return SDL_SCANCODE_MINUS;
    case 190: return SDL_SCANCODE_PERIOD;
    case 191: return SDL_SCANCODE_SLASH;
    case 192: return SDL_SCANCODE_GRAVE;
    case 219: return SDL_SCANCODE_LEFTBRACKET;
    case 220: return SDL_SCANCODE_BACKSLASH;
    case 221: return SDL_SCANCODE_RIGHTBRACKET;
    case 222: return SDL_SCANCODE_APOSTROPHE;
    default: return SDL_SCANCODE_UNKNOWN;
    }
}

int scancodeToFlashKey(SDL_Scancode sc)
{
    switch (sc) {
    case SDL_SCANCODE_RSHIFT: return 16;
    case SDL_SCANCODE_RCTRL: return 17;
    case SDL_SCANCODE_RALT: return 18;
    case SDL_SCANCODE_KP_ENTER: return 13;
    default: break;
    }
    for (int c = 1; c < 256; ++c) {
        if (flashKeyToScancode(c) == sc) {
            return c;
        }
    }
    return 0;
}

ActionState Input::poll() const
{
    ActionState s;
    const bool *k = SDL_GetKeyboardState(nullptr);
    // The game's keys (the original's defaults: arrows, Up also jumps, Space attacks; the CONTROLS tab changes them).
    // Port extras, while no game key uses them: W A S D, Z (jump) and X (attack).
    auto held = [&](int code) {
        const SDL_Scancode sc = flashKeyToScancode(code);
        return sc != SDL_SCANCODE_UNKNOWN && k[sc];
    };
    auto extra = [&](SDL_Scancode sc) {
        for (int b : keyBindings) {
            if (flashKeyToScancode(b) == sc) {
                return false;
            }
        }
        return k[sc];
    };
    if (held(keyBindings[0]) || extra(SDL_SCANCODE_A)) set(s, Action::Left);
    if (held(keyBindings[1]) || extra(SDL_SCANCODE_D)) set(s, Action::Right);
    if (held(keyBindings[2])) set(s, Action::Up);
    if (held(keyBindings[3]) || extra(SDL_SCANCODE_S)) set(s, Action::Down);
    if (held(keyBindings[4])) set(s, Action::Jump);
    if (extra(SDL_SCANCODE_W) || extra(SDL_SCANCODE_Z)) {
        set(s, Action::Up);
        set(s, Action::Jump);
    }
    if (held(keyBindings[5]) || extra(SDL_SCANCODE_X)) set(s, Action::Attack);
    if (gamepad_) {
        const float lx = SDL_GetGamepadAxis(gamepad_, SDL_GAMEPAD_AXIS_LEFTX) / 32767.0f;
        const float ly = SDL_GetGamepadAxis(gamepad_, SDL_GAMEPAD_AXIS_LEFTY) / 32767.0f;
        if (SDL_GetGamepadButton(gamepad_, SDL_GAMEPAD_BUTTON_DPAD_LEFT) || lx < -0.4f) set(s, Action::Left);
        if (SDL_GetGamepadButton(gamepad_, SDL_GAMEPAD_BUTTON_DPAD_RIGHT) || lx > 0.4f) set(s, Action::Right);
        if (SDL_GetGamepadButton(gamepad_, SDL_GAMEPAD_BUTTON_DPAD_DOWN) || ly > 0.6f) set(s, Action::Down);
        if (SDL_GetGamepadButton(gamepad_, SDL_GAMEPAD_BUTTON_DPAD_UP) || ly < -0.6f) set(s, Action::Up);
        if (SDL_GetGamepadButton(gamepad_, SDL_GAMEPAD_BUTTON_SOUTH)) {
            set(s, Action::Jump);
            set(s, Action::Up);
        }
        if (SDL_GetGamepadButton(gamepad_, SDL_GAMEPAD_BUTTON_WEST) || SDL_GetGamepadButton(gamepad_, SDL_GAMEPAD_BUTTON_EAST)) {
            set(s, Action::Attack);
        }
    }
    touch.held(s);
    return s;
}

} // namespace engine
