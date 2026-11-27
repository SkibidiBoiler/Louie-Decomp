#include "engine/launcher.h"

#include <algorithm>
#include <cmath>

#include "engine/import_screen.h"
#include "engine/lang.h"
#include "engine/math.h"
#include "engine/paths.h"
#include "engine/text.h"
#include "engine/ui_draw.h"

namespace engine {

namespace {

// The cover banner is the title screen's strip y 72..296 (700 x 224); a box shows its middle, x 200..500.
constexpr float kBoxAspect = 300.0f / 224.0f;
constexpr float kBoxU0 = 200.0f / 700.0f, kBoxU1 = 500.0f / 700.0f;
constexpr float kSmallBox = 0.78f; // the boxes that are not selected
constexpr float kFadeSeconds = 0.35f;
constexpr SDL_FColor kInk{0.05f, 0.07f, 0.12f, 1}; // text on a bright button

// Interface text in the embedded font; before any game is imported there is none, so SDL's debug font
// (ASCII only) stands in.
float label(SDL_Renderer *r, TextLibrary &text, float size, const std::string &s, float x, float baseline,
            SDL_FColor color, int align, bool draw = true)
{
    if (text.drawString(ui::kFont, size, "A", 0, 0, color, 0, Affine{}, false) > 0) {
        return text.drawString(ui::kFont, size, s, x, baseline, color, align, Affine{}, draw);
    }
    const float scale = std::max(1.0f, std::floor(size / 10));
    const float w = static_cast<float>(s.size()) * 8 * scale;
    if (draw) {
        const float left = align == 1 ? x - w : (align == 2 ? x - w / 2 : x);
        SDL_SetRenderDrawColorFloat(r, color.r, color.g, color.b, color.a);
        SDL_SetRenderScale(r, scale, scale);
        SDL_RenderDebugText(r, left / scale, (baseline - 7 * scale) / scale, s.c_str());
        SDL_SetRenderScale(r, 1, 1);
    }
    return w;
}

// One line, shrunk to fit `maxW`.
void labelFit(SDL_Renderer *r, TextLibrary &text, float size, const std::string &s, float x, float baseline,
              SDL_FColor color, int align, float maxW)
{
    const float w = label(r, text, size, s, 0, 0, color, 0, false);
    if (w > maxW && w > 0) {
        size *= maxW / w;
    }
    label(r, text, size, s, x, baseline, color, align);
}

void gradient(SDL_Renderer *r, const SDL_FRect &rc, SDL_FColor top, SDL_FColor bottom)
{
    const SDL_Vertex v[4] = {{{rc.x, rc.y}, top, {0, 0}},
                             {{rc.x + rc.w, rc.y}, top, {0, 0}},
                             {{rc.x + rc.w, rc.y + rc.h}, bottom, {0, 0}},
                             {{rc.x, rc.y + rc.h}, bottom, {0, 0}}};
    const int idx[6] = {0, 1, 2, 0, 2, 3};
    SDL_RenderGeometry(r, nullptr, v, 4, idx, 6);
}

SDL_FColor shade(SDL_FColor c, float k, float a = 1)
{
    return {std::min(1.0f, c.r * k), std::min(1.0f, c.g * k), std::min(1.0f, c.b * k), a};
}

bool inside(const SDL_FRect &rc, float x, float y)
{
    return x >= rc.x && x < rc.x + rc.w && y >= rc.y && y < rc.y + rc.h;
}

} // namespace

Launcher::Launcher(Lang &lang, ImportScreen &import, std::string dataDir)
    : lang_(lang), import_(import), dataDir_(std::move(dataDir))
{
    refresh();
}

Launcher::~Launcher()
{
    clearCovers();
}

void Launcher::refresh()
{
    for (Game &g : games_) {
        g.imported = fileExists(dataDir_ + "/" + g.id + "/symbols.bin");
    }
}

bool Launcher::imported(const std::string &gameId) const
{
    for (const Game &g : games_) {
        if (gameId == g.id) {
            return g.imported;
        }
    }
    return false;
}

void Launcher::select(const std::string &gameId)
{
    for (int i = 0; i < GameCount; ++i) {
        if (gameId == games_[i].id) {
            move(i - selected_);
        }
    }
}

void Launcher::setCover(const std::string &gameId, SDL_Texture *banner)
{
    for (Game &g : games_) {
        if (gameId == g.id) {
            if (g.cover) {
                SDL_DestroyTexture(g.cover);
            }
            g.cover = banner;
            return;
        }
    }
    SDL_DestroyTexture(banner);
}

bool Launcher::hasCover(const std::string &gameId) const
{
    for (const Game &g : games_) {
        if (gameId == g.id) {
            return g.cover != nullptr;
        }
    }
    return false;
}

void Launcher::clearCovers()
{
    for (Game &g : games_) {
        if (g.cover) {
            SDL_DestroyTexture(g.cover);
            g.cover = nullptr;
        }
    }
}

std::string Launcher::takeChosen()
{
    std::string id;
    id.swap(chosen_);
    return id;
}

void Launcher::update(float dt)
{
    fade_ = std::min(1.0f, fade_ + dt / kFadeSeconds);
    const float k = std::min(1.0f, dt * 14);
    for (int i = 0; i < GameCount; ++i) {
        const float target = i == selected_ ? 1.0f : kSmallBox;
        scale_[i] += (target - scale_[i]) * k;
    }
}

void Launcher::move(int delta)
{
    focus_ = Focus::Games;
    const int next = std::clamp(selected_ + delta, 0, GameCount - 1);
    if (next == selected_) {
        return;
    }
    // A fade that is still running restarts from the game shown most.
    previous_ = fade_ < 0.5f ? previous_ : selected_;
    selected_ = next;
    fade_ = 0;
}

Launcher::Layout Launcher::layout(int w, int h) const
{
    Layout l;
    const float fw = static_cast<float>(w), fh = static_cast<float>(h);
    l.pad = fh * 0.06f;
    const float boxTop = fh * 0.16f, boxH = fh * 0.32f, boxW = boxH * kBoxAspect, gap = fh * 0.035f;
    float total = gap * (GameCount - 1);
    for (float s : scale_) {
        total += boxW * s;
    }
    const float f = std::min(1.0f, (fw - 2 * l.pad) / total);
    float x = l.pad;
    for (int i = 0; i < GameCount; ++i) {
        l.boxes[i] = {x, boxTop, boxW * scale_[i] * f, boxH * scale_[i] * f};
        x += l.boxes[i].w + gap * f;
    }
    l.titleSize = fh * 0.058f;
    l.infoY = boxTop + boxH * f + fh * 0.1f;
    l.action = {l.pad, l.infoY + fh * 0.1f, fh * 0.44f, fh * 0.09f};
    l.quit = {l.pad, fh - l.pad - fh * 0.075f, fh * 0.26f, fh * 0.075f};
    return l;
}

Launcher::Target Launcher::hit(float x, float y, int w, int h, int &box) const
{
    const Layout l = layout(w, h);
    box = -1;
    for (int i = 0; i < GameCount; ++i) {
        if (inside(l.boxes[i], x, y)) {
            box = i;
            return Target::Box;
        }
    }
    if (inside(l.action, x, y) && !actionText(games_[selected_]).empty()) {
        return Target::Action;
    }
    return inside(l.quit, x, y) ? Target::Quit : Target::None;
}

void Launcher::pointerMove(float x, float y, int w, int h)
{
    int box = -1;
    switch (hit(x, y, w, h, box)) {
    case Target::Action: focus_ = Focus::Games; break;
    case Target::Quit: focus_ = Focus::Quit; break;
    default: break;
    }
}

void Launcher::pointerDown(float x, float y, int w, int h)
{
    pressed_ = hit(x, y, w, h, pressedBox_);
}

void Launcher::pointerUp(float x, float y, int w, int h)
{
    int box = -1;
    const Target t = hit(x, y, w, h, box);
    if (t != Target::None && t == pressed_ && box == pressedBox_) {
        switch (t) {
        case Target::Box:
            if (box == selected_) {
                focus_ = Focus::Games;
                activate(); // a second tap on the selected box starts it
            } else {
                move(box - selected_);
            }
            break;
        case Target::Action:
            focus_ = Focus::Games;
            activate();
            break;
        case Target::Quit:
            quit_ = true;
            break;
        default:
            break;
        }
    }
    pressed_ = Target::None;
    pressedBox_ = -1;
}

bool Launcher::key(SDL_Keycode k)
{
    switch (k) {
    case SDLK_LEFT:
    case SDLK_A:
        move(-1);
        return true;
    case SDLK_RIGHT:
    case SDLK_D:
        move(1);
        return true;
    case SDLK_DOWN:
    case SDLK_S:
        focus_ = Focus::Quit;
        return true;
    case SDLK_UP:
    case SDLK_W:
        focus_ = Focus::Games;
        return true;
    case SDLK_RETURN:
    case SDLK_KP_ENTER:
    case SDLK_SPACE:
        activate();
        return true;
    default:
        return false;
    }
}

void Launcher::activate()
{
    if (focus_ == Focus::Quit) {
        quit_ = true;
        return;
    }
    const Game &g = games_[selected_];
    if (!g.imported) {
        import_.pick(g.id); // ignored while another import runs
    } else if (g.ported) {
        chosen_ = g.id;
    }
}

std::string Launcher::statusText(const Game &g) const
{
    if (g.ported) {
        return lang_.get(g.imported ? "launcher.ready" : "launcher.not_imported");
    }
    std::string s = lang_.get("launcher.coming_soon");
    if (!g.imported) {
        s += " - " + lang_.get("launcher.not_imported");
    }
    return s;
}

std::string Launcher::actionText(const Game &g) const
{
    if (!g.imported) {
        return lang_.get("launcher.import");
    }
    return g.ported ? lang_.get("launcher.play") : std::string();
}

void Launcher::renderBackground(SDL_Renderer *r, int w, int h) const
{
    const float fw = static_cast<float>(w), fh = static_cast<float>(h);
    const SDL_FRect all{0, 0, fw, fh};
    auto draw = [&](const Game &g, float alpha) {
        if (alpha <= 0) {
            return;
        }
        if (g.cover) {
            // The title art filling the screen (cropped to the window's shape).
            float tw = 0, th = 0;
            SDL_GetTextureSize(g.cover, &tw, &th);
            SDL_FRect src{0, 0, tw, th};
            if (tw / th > fw / fh) {
                src.w = th * fw / fh;
                src.x = (tw - src.w) / 2;
            } else {
                src.h = tw * fh / fw;
                src.y = (th - src.h) / 2;
            }
            SDL_SetTextureAlphaModFloat(g.cover, alpha);
            SDL_RenderTexture(r, g.cover, &src, &all);
            SDL_SetTextureAlphaModFloat(g.cover, 1);
        } else {
            gradient(r, all, shade(g.theme, 0.9f, alpha), shade(g.theme, 0.3f, alpha));
        }
    };
    draw(games_[previous_], 1);
    draw(games_[selected_], fade_);
    // Darker towards the bottom and the left, where the text is; a touch of the game's colour.
    gradient(r, all, SDL_FColor{0, 0, 0, 0.42f}, SDL_FColor{0, 0, 0, 0.85f});
    const SDL_Vertex v[4] = {{{0, 0}, SDL_FColor{0, 0, 0, 0.45f}, {0, 0}},
                             {{fw * 0.6f, 0}, SDL_FColor{0, 0, 0, 0}, {0, 0}},
                             {{fw * 0.6f, fh}, SDL_FColor{0, 0, 0, 0}, {0, 0}},
                             {{0, fh}, SDL_FColor{0, 0, 0, 0.45f}, {0, 0}}};
    const int idx[6] = {0, 1, 2, 0, 2, 3};
    SDL_RenderGeometry(r, nullptr, v, 4, idx, 6);
    ui::fillRect(r, 0, 0, fw, fh, shade(games_[selected_].theme, 1, 0.12f));
}

void Launcher::renderBox(SDL_Renderer *r, TextLibrary &text, int i, const SDL_FRect &box) const
{
    const Game &g = games_[i];
    const bool selected = i == selected_;
    if (selected) {
        // A soft glow around the selected box.
        for (int k = 3; k >= 1; --k) {
            const float e = box.h * 0.018f * static_cast<float>(k);
            ui::fillRect(r, box.x - e, box.y - e, box.w + 2 * e, box.h + 2 * e, SDL_FColor{1, 1, 1, 0.06f});
        }
    } else {
        ui::fillRect(r, box.x + box.h * 0.02f, box.y + box.h * 0.03f, box.w, box.h, SDL_FColor{0, 0, 0, 0.35f});
    }
    if (g.cover) {
        float tw = 0, th = 0;
        SDL_GetTextureSize(g.cover, &tw, &th);
        const SDL_FRect src{tw * kBoxU0, 0, tw * (kBoxU1 - kBoxU0), th};
        SDL_RenderTexture(r, g.cover, &src, &box);
    } else {
        // No art without the game's data: its colours and its name.
        gradient(r, box, shade(g.theme, 1.25f), shade(g.theme, 0.55f));
        const std::string id = g.id;
        labelFit(r, text, box.h * 0.17f, lang_.get("launcher.short_" + id), box.x + box.w / 2, box.y + box.h * 0.5f,
                 ui::kTextBright, 2, box.w * 0.86f);
        labelFit(r, text, box.h * 0.08f, lang_.get("launcher.sub_" + id), box.x + box.w / 2, box.y + box.h * 0.66f,
                 ui::kTextDim, 2, box.w * 0.86f);
    }
    if (!selected) {
        ui::fillRect(r, box.x, box.y, box.w, box.h, SDL_FColor{0, 0, 0, 0.35f});
    }
    ui::frameRect(r, box.x, box.y, box.w, box.h, std::max(2.0f, box.h * (selected ? 0.014f : 0.006f)),
                  SDL_FColor{1, 1, 1, selected ? 0.95f : 0.35f});
}

void Launcher::render(SDL_Renderer *r, TextLibrary &text, int w, int h) const
{
    const Layout l = layout(w, h);
    const float fw = static_cast<float>(w), fh = static_cast<float>(h);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    renderBackground(r, w, h);

    label(r, text, fh * 0.034f, lang_.get("launcher.title"), l.pad, l.pad + fh * 0.02f, ui::kTextDim, 0);
    const std::string version = std::string("v") + PL_VERSION + (PL_COMMIT[0] ? std::string(" (") + PL_COMMIT + ")" : "");
    label(r, text, fh * 0.026f, version, l.pad, l.pad + fh * 0.065f, ui::kTextDim, 0);
    // The selected box last, so its glow is on top.
    for (int i = 0; i < GameCount; ++i) {
        if (i != selected_) {
            renderBox(r, text, i, l.boxes[i]);
        }
    }
    renderBox(r, text, selected_, l.boxes[selected_]);

    const Game &g = games_[selected_];
    const float maxW = fw - 2 * l.pad;
    labelFit(r, text, l.titleSize, lang_.get(std::string("launcher.game_") + g.id), l.pad, l.infoY, ui::kTextBright, 0,
             maxW);
    label(r, text, fh * 0.03f, statusText(g), l.pad, l.infoY + fh * 0.05f, ui::kTextDim, 0);

    // The action button, or the import's progress in its place.
    const ImportScreen::Status st = import_.status();
    const float btnText = l.action.h * 0.36f;
    if (st.running && st.gameId == g.id) {
        ui::pill(r, l.action, SDL_FColor{1, 1, 1, 0.16f});
        SDL_FRect fill = l.action;
        fill.w = std::max(fill.h, fill.w * std::clamp(st.fraction, 0.0f, 1.0f));
        ui::pill(r, fill, SDL_FColor{1, 1, 1, 0.5f});
        label(r, text, btnText, lang_.get("launcher.importing_short", std::to_string(static_cast<int>(st.fraction * 100))),
              l.action.x + l.action.w / 2, l.action.y + l.action.h * 0.63f, ui::kTextBright, 2);
    } else if (const std::string action = actionText(g); !action.empty()) {
        const bool focused = focus_ == Focus::Games;
        ui::pill(r, l.action, focused ? SDL_FColor{1, 1, 1, 0.95f} : SDL_FColor{1, 1, 1, 0.16f});
        label(r, text, btnText, action, l.action.x + l.action.w / 2, l.action.y + l.action.h * 0.63f,
              focused ? kInk : ui::kTextBright, 2);
    }
    if (st.failed && st.gameId == g.id) {
        labelFit(r, text, fh * 0.026f, lang_.get("launcher.import_failed") + " " + st.error, l.pad,
                 l.action.y + l.action.h + fh * 0.045f, SDL_FColor{1, 0.6f, 0.5f, 0.95f}, 0, maxW);
    }

    const bool quitFocused = focus_ == Focus::Quit;
    ui::pill(r, l.quit, quitFocused ? SDL_FColor{1, 1, 1, 0.95f} : SDL_FColor{1, 1, 1, 0.12f});
    label(r, text, l.quit.h * 0.38f, lang_.get("launcher.quit"), l.quit.x + l.quit.w / 2, l.quit.y + l.quit.h * 0.64f,
          quitFocused ? kInk : ui::kTextBright, 2);
    labelFit(r, text, fh * 0.022f, lang_.get("launcher.note"), fw - l.pad, l.quit.y + l.quit.h * 0.64f,
             SDL_FColor{1, 1, 1, 0.5f}, 1, fw - 3 * l.pad - l.quit.w);
}

} // namespace engine
