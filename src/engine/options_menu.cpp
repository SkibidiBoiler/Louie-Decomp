#include "engine/options_menu.h"

#include <algorithm>
#include <cmath>
#include <string>

#include "engine/input.h"
#include "engine/lang.h"
#include "engine/math.h"
#include "engine/settings.h"
#include "engine/text.h"
#include "engine/ui_draw.h"

namespace engine {

namespace {

constexpr float kOpacities[] = {0.25f, 0.4f, 0.55f, 0.7f, 0.85f};
constexpr float kSizes[] = {0.75f, 1.0f, 1.25f, 1.5f};
constexpr const char *kLanguages[] = {"", "en", "es"};

using ui::disc;
using ui::fillRect;
using ui::kFont;
using ui::ring;

template <size_t N> int indexOf(const float (&values)[N], float v)
{
    int best = 0;
    for (size_t i = 0; i < N; ++i) {
        if (std::fabs(values[i] - v) < std::fabs(values[best] - v)) {
            best = static_cast<int>(i);
        }
    }
    return best;
}

int wrap(int i, int n) { return ((i % n) + n) % n; }

} // namespace

void OptionsMenu::setOpen(bool open)
{
    open_ = open;
    gearPressed_ = false;
    if (open) {
        selected_ = 0;
    }
}

void OptionsMenu::applySettings()
{
    touch_.opacity = settings_.touchOpacity;
    touch_.size = settings_.touchSize;
}

OptionsMenu::Layout OptionsMenu::layout(int w, int h) const
{
    Layout l;
    const float unit = static_cast<float>(std::min(w, h));
    l.gearR = unit * 0.05f;
    l.gearX = static_cast<float>(w) - l.gearR * 1.5f;
    l.gearY = l.gearR * 1.5f;
    l.rowH = unit * 0.095f;
    l.titleH = l.rowH * 1.2f;
    l.panelW = std::min(static_cast<float>(w) * 0.8f, unit * 1.45f);
    const float panelH = l.titleH + l.rowH * static_cast<float>(RowCount);
    l.panelX = (static_cast<float>(w) - l.panelW) / 2;
    l.panelY = (static_cast<float>(h) - panelH) / 2;
    return l;
}

int OptionsMenu::rowAt(float x, float y, const Layout &l) const
{
    if (x < l.panelX || x > l.panelX + l.panelW) {
        return -1;
    }
    const float top = l.panelY + l.titleH;
    if (y < top || y >= top + l.rowH * static_cast<float>(RowCount)) {
        return y >= l.panelY && y < top ? -2 : -1; // -2: the title bar (inside the panel)
    }
    return static_cast<int>((y - top) / l.rowH);
}

bool OptionsMenu::pointerDown(float x, float y, int w, int h)
{
    const Layout l = layout(w, h);
    if (!open_) {
        gearPressed_ = std::hypot(x - l.gearX, y - l.gearY) <= l.gearR * 1.25f;
        return gearPressed_;
    }
    return true;
}

bool OptionsMenu::pointerUp(float x, float y, int w, int h)
{
    const Layout l = layout(w, h);
    if (!open_) {
        if (gearPressed_) {
            gearPressed_ = false;
            if (std::hypot(x - l.gearX, y - l.gearY) <= l.gearR * 1.25f) {
                setOpen(true);
            }
            return true;
        }
        return false;
    }
    if (std::hypot(x - l.gearX, y - l.gearY) <= l.gearR * 1.25f) {
        setOpen(false);
        return true;
    }
    const int row = rowAt(x, y, l);
    if (row == -1) {
        setOpen(false); // a tap outside the panel closes it
    } else if (row >= 0) {
        selected_ = row;
        // The left third of a row steps back, the rest steps forward.
        change(row, x < l.panelX + l.panelW / 3 ? -1 : 1);
    }
    return true;
}

bool OptionsMenu::key(SDL_Keycode k)
{
    if (k == SDLK_F1) {
        setOpen(!open_);
        return true;
    }
    if (!open_) {
        return false;
    }
    switch (k) {
    case SDLK_ESCAPE:
    case SDLK_AC_BACK:
        setOpen(false);
        break;
    case SDLK_UP:
    case SDLK_W:
        selected_ = wrap(selected_ - 1, RowCount);
        break;
    case SDLK_DOWN:
    case SDLK_S:
        selected_ = wrap(selected_ + 1, RowCount);
        break;
    case SDLK_LEFT:
    case SDLK_A:
        change(selected_, -1);
        break;
    case SDLK_RIGHT:
    case SDLK_D:
    case SDLK_RETURN:
    case SDLK_SPACE:
        change(selected_, 1);
        break;
    default:
        break;
    }
    return true;
}

void OptionsMenu::change(int row, int dir)
{
    switch (row) {
    case Fps: {
        const int values[] = {originalRate_, 60, 120, 0};
        int i = 0;
        for (int j = 0; j < 4; ++j) {
            if (values[j] == settings_.fpsCap) {
                i = j;
            }
        }
        settings_.fpsCap = values[wrap(i + dir, 4)];
        break;
    }
    case Interpolation:
        settings_.interpolation = !settings_.interpolation;
        break;
    case Widescreen:
        settings_.widescreen = !settings_.widescreen;
        break;
    case Language: {
        int i = 0;
        for (int j = 0; j < 3; ++j) {
            if (settings_.language == kLanguages[j]) {
                i = j;
            }
        }
        settings_.language = kLanguages[wrap(i + dir, 3)];
        lang_.select(settings_.language);
        break;
    }
    case TouchOpacity:
        settings_.touchOpacity = kOpacities[wrap(indexOf(kOpacities, settings_.touchOpacity) + dir, 5)];
        break;
    case TouchSize:
        settings_.touchSize = kSizes[wrap(indexOf(kSizes, settings_.touchSize) + dir, 4)];
        break;
    case Overlay:
        settings_.showOverlay = !settings_.showOverlay;
        break;
    case IntroMusic:
        settings_.waitIntroMusic = !settings_.waitIntroMusic;
        break;
    case Close:
        setOpen(false);
        return;
    default:
        return;
    }
    applySettings();
    settings_.save();
}

const char *OptionsMenu::labelKey(int row) const
{
    switch (row) {
    case Fps: return "options.fps";
    case Interpolation: return "options.interpolation";
    case Widescreen: return "options.widescreen";
    case Language: return "options.language";
    case TouchOpacity: return "options.touch_opacity";
    case TouchSize: return "options.touch_size";
    case Overlay: return "options.overlay";
    case IntroMusic: return "options.intro_music";
    default: return "options.close";
    }
}

std::string OptionsMenu::valueText(int row) const
{
    switch (row) {
    case Fps:
        if (settings_.fpsCap == 0) {
            return lang_.get("options.fps_max");
        }
        if (settings_.fpsCap == originalRate_) {
            return lang_.get("options.fps_original", std::to_string(originalRate_));
        }
        return std::to_string(settings_.fpsCap);
    case Interpolation:
    case Widescreen:
    case Overlay:
    case IntroMusic: {
        const bool on = row == Interpolation ? settings_.interpolation
                        : row == Widescreen  ? settings_.widescreen
                        : row == Overlay     ? settings_.showOverlay
                                             : settings_.waitIntroMusic;
        return lang_.get(on ? "options.on" : "options.off");
    }
    case Language:
        if (settings_.language.empty()) {
            return lang_.get("options.language_system");
        }
        return settings_.language == "es" ? "Español" : "English";
    case TouchOpacity:
        return std::to_string(static_cast<int>(std::lround((1.0f - settings_.touchOpacity) * 100))) + " %";
    case TouchSize:
        return std::to_string(static_cast<int>(std::lround(settings_.touchSize * 100))) + " %";
    default:
        return {};
    }
}

void OptionsMenu::render(SDL_Renderer *r, TextLibrary &text, int w, int h) const
{
    const Layout l = layout(w, h);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    // The gear button, like the touch buttons: a dark translucent disc with a white ring and icon.
    const float o = 0.75f;
    disc(r, l.gearX, l.gearY, l.gearR, SDL_FColor{0.05f, 0.07f, 0.12f, (open_ ? 0.45f : 0.22f) * o});
    ring(r, l.gearX, l.gearY, l.gearR, std::max(2.0f, l.gearR * 0.08f), SDL_FColor{1, 1, 1, 0.6f * o});
    {
        const SDL_FColor icon{1, 1, 1, 0.8f * o};
        const float tooth = l.gearR * 0.16f;
        for (int i = 0; i < 8; ++i) {
            const float a = static_cast<float>(i) / 8 * 6.2831853f;
            disc(r, l.gearX + std::cos(a) * l.gearR * 0.5f, l.gearY + std::sin(a) * l.gearR * 0.5f, tooth, icon);
        }
        ring(r, l.gearX, l.gearY, l.gearR * 0.48f, l.gearR * 0.2f, icon);
    }
    if (!open_) {
        return;
    }
    // Dim the game a little, then the panel.
    fillRect(r, 0, 0, static_cast<float>(w), static_cast<float>(h), SDL_FColor{0, 0, 0, 0.25f});
    const float panelH = l.titleH + l.rowH * static_cast<float>(RowCount);
    ui::panel(r, l.panelX, l.panelY, l.panelW, panelH, l.rowH);
    const SDL_FColor white = ui::kTextBright;
    const SDL_FColor dim = ui::kTextDim;
    const float titleSize = l.titleH * 0.5f;
    text.drawString(kFont, titleSize, lang_.get("options.title"), l.panelX + l.panelW / 2,
                    l.panelY + l.titleH * 0.68f, white, 2, Affine{});
    const float size = l.rowH * 0.36f;
    const float pad = l.rowH * 0.35f;
    for (int i = 0; i < RowCount; ++i) {
        const float y = l.panelY + l.titleH + l.rowH * static_cast<float>(i);
        if (i == selected_) {
            fillRect(r, l.panelX + 4, y + 2, l.panelW - 8, l.rowH - 4, ui::kRowHighlight);
        } else {
            fillRect(r, l.panelX + pad, y, l.panelW - 2 * pad, 1, ui::kRowSeparator);
        }
        const float baseline = y + l.rowH * 0.64f;
        if (i == Close) {
            text.drawString(kFont, size, lang_.get(labelKey(i)), l.panelX + l.panelW / 2, baseline, white, 2,
                            Affine{});
            continue;
        }
        text.drawString(kFont, size, lang_.get(labelKey(i)), l.panelX + pad, baseline, dim, 0, Affine{});
        const std::string value = "<  " + valueText(i) + "  >";
        text.drawString(kFont, size, value, l.panelX + l.panelW - pad, baseline, white, 1, Affine{});
    }
}

} // namespace engine
