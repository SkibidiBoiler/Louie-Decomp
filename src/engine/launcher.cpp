#include "engine/launcher.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#include "engine/import_screen.h"
#include "engine/lang.h"
#include "engine/math.h"
#include "engine/paths.h"
#include "engine/text.h"
#include "engine/ui_draw.h"

namespace engine {

namespace {

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

// A vertical gradient over the whole window, behind the panel.
void background(SDL_Renderer *r, int w, int h)
{
    const SDL_FColor top{0.11f, 0.15f, 0.25f, 1}, bottom{0.03f, 0.04f, 0.08f, 1};
    const float fw = static_cast<float>(w), fh = static_cast<float>(h);
    const SDL_Vertex v[4] = {{{0, 0}, top, {0, 0}}, {{fw, 0}, top, {0, 0}}, {{fw, fh}, bottom, {0, 0}},
                             {{0, fh}, bottom, {0, 0}}};
    const int idx[6] = {0, 1, 2, 0, 2, 3};
    SDL_RenderGeometry(r, nullptr, v, 4, idx, 6);
}

int wrap(int i, int n) { return ((i % n) + n) % n; }

} // namespace

Launcher::Launcher(Lang &lang, ImportScreen &import, std::string dataDir)
    : lang_(lang), import_(import), dataDir_(std::move(dataDir))
{
    refresh();
}

void Launcher::refresh()
{
    for (Game &g : games_) {
        g.imported = fileExists(dataDir_ + "/" + g.id + "/symbols.bin");
    }
}

void Launcher::select(const std::string &gameId)
{
    for (int i = 0; i < GameCount; ++i) {
        if (gameId == games_[i].id) {
            selected_ = i;
        }
    }
}

std::string Launcher::takeChosen()
{
    std::string id;
    id.swap(chosen_);
    return id;
}

Launcher::Layout Launcher::layout(int w, int h) const
{
    Layout l;
    const float unit = static_cast<float>(std::min(w, h));
    l.rowH = unit * 0.135f;
    l.titleH = unit * 0.2f;
    l.quitH = l.rowH * 0.7f;
    l.statusH = l.rowH * 0.6f;
    l.panelW = std::min(static_cast<float>(w) * 0.88f, unit * 1.6f);
    l.panelH = l.titleH + l.rowH * static_cast<float>(GameCount) + l.quitH + l.statusH;
    l.panelX = (static_cast<float>(w) - l.panelW) / 2;
    l.panelY = (static_cast<float>(h) - l.panelH) / 2;
    return l;
}

int Launcher::rowAt(float x, float y, const Layout &l) const
{
    if (x < l.panelX || x > l.panelX + l.panelW) {
        return -1;
    }
    const float top = l.panelY + l.titleH;
    if (y >= top && y < top + l.rowH * static_cast<float>(GameCount)) {
        return static_cast<int>((y - top) / l.rowH);
    }
    const float quitTop = top + l.rowH * static_cast<float>(GameCount);
    return y >= quitTop && y < quitTop + l.quitH ? QuitRow : -1;
}

void Launcher::pointerMove(float x, float y, int w, int h)
{
    const int row = rowAt(x, y, layout(w, h));
    if (row >= 0) {
        selected_ = row;
    }
}

void Launcher::pointerDown(float x, float y, int w, int h)
{
    pressed_ = rowAt(x, y, layout(w, h));
    if (pressed_ >= 0) {
        selected_ = pressed_;
    }
}

void Launcher::pointerUp(float x, float y, int w, int h)
{
    const int row = rowAt(x, y, layout(w, h));
    if (row >= 0 && row == pressed_) {
        activate(row);
    }
    pressed_ = -1;
}

bool Launcher::key(SDL_Keycode k)
{
    switch (k) {
    case SDLK_UP:
    case SDLK_W:
        selected_ = wrap(selected_ - 1, RowCount);
        return true;
    case SDLK_DOWN:
    case SDLK_S:
        selected_ = wrap(selected_ + 1, RowCount);
        return true;
    case SDLK_RETURN:
    case SDLK_KP_ENTER:
    case SDLK_SPACE:
        activate(selected_);
        return true;
    default:
        return false;
    }
}

void Launcher::activate(int row)
{
    if (row == QuitRow) {
        quit_ = true;
        return;
    }
    const Game &g = games_[row];
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
    const ImportScreen::Status st = import_.status();
    if (st.running && st.gameId == g.id) {
        return std::to_string(static_cast<int>(st.fraction * 100)) + " %";
    }
    if (!g.imported) {
        return lang_.get("launcher.import");
    }
    return g.ported ? lang_.get("launcher.play") : std::string();
}

void Launcher::render(SDL_Renderer *r, TextLibrary &text, int w, int h) const
{
    const Layout l = layout(w, h);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    background(r, w, h);
    ui::panel(r, l.panelX, l.panelY, l.panelW, l.panelH, l.rowH);
    const float pad = l.rowH * 0.3f;
    const float cx = l.panelX + l.panelW / 2;

    label(r, text, l.titleH * 0.3f, lang_.get("launcher.title"), cx, l.panelY + l.titleH * 0.47f, ui::kTextBright, 2);
    label(r, text, l.titleH * 0.16f, lang_.get("launcher.subtitle"), cx, l.panelY + l.titleH * 0.8f, ui::kTextDim, 2);

    const ImportScreen::Status st = import_.status();
    for (int i = 0; i < RowCount; ++i) {
        const bool quit = i == QuitRow;
        const float y = l.panelY + l.titleH + l.rowH * static_cast<float>(i);
        const float rowH = quit ? l.quitH : l.rowH;
        if (i == selected_) {
            ui::fillRect(r, l.panelX + 4, y + 2, l.panelW - 8, rowH - 4, ui::kRowHighlight);
        } else {
            ui::fillRect(r, l.panelX + pad, y, l.panelW - 2 * pad, 1, ui::kRowSeparator);
        }
        if (quit) {
            label(r, text, l.rowH * 0.27f, lang_.get("launcher.quit"), cx, y + rowH * 0.66f, ui::kTextBright, 2);
            continue;
        }
        const Game &g = games_[i];
        const bool playable = g.ported && g.imported;
        // The action on the right; the title on the left, shrunk if it would run into it.
        const std::string action = actionText(g);
        const float actionSize = l.rowH * 0.25f;
        const float actionW = action.empty() ? 0 : label(r, text, actionSize, action, 0, 0, ui::kTextBright, 0, false);
        if (!action.empty()) {
            label(r, text, actionSize, action, l.panelX + l.panelW - pad, y + l.rowH * 0.6f, ui::kTextBright, 1);
        }
        const std::string title = lang_.get(std::string("launcher.game_") + g.id);
        float titleSize = l.rowH * 0.3f;
        const float room = l.panelW - 3 * pad - actionW;
        const float titleW = label(r, text, titleSize, title, 0, 0, ui::kTextBright, 0, false);
        if (titleW > room && titleW > 0) {
            titleSize *= room / titleW;
        }
        label(r, text, titleSize, title, l.panelX + pad, y + l.rowH * 0.48f, playable ? ui::kTextBright : ui::kTextDim, 0);
        label(r, text, l.rowH * 0.19f, statusText(g), l.panelX + pad, y + l.rowH * 0.8f, ui::kTextDim, 0);
    }

    // The status line: the import's progress, its error, or the project's note.
    const float sy = l.panelY + l.panelH - l.statusH;
    const float size = l.statusH * 0.3f;
    if (st.running) {
        const float barW = l.panelW * 0.5f, barH = l.statusH * 0.16f;
        const float bx = cx - barW / 2, by = sy + l.statusH * 0.62f;
        ui::fillRect(r, bx, by, barW, barH, SDL_FColor{1, 1, 1, 0.18f});
        ui::fillRect(r, bx, by, barW * std::clamp(st.fraction, 0.0f, 1.0f), barH, ui::kTextBright);
        label(r, text, size, lang_.get("launcher.importing", lang_.get("launcher.game_" + st.gameId)), cx,
              sy + l.statusH * 0.45f, ui::kTextDim, 2);
    } else if (st.failed) {
        std::string msg = lang_.get("launcher.import_failed") + " " + st.error;
        float s = size;
        const float wMsg = label(r, text, s, msg, 0, 0, ui::kTextBright, 0, false);
        if (wMsg > l.panelW - 2 * pad && wMsg > 0) {
            s *= (l.panelW - 2 * pad) / wMsg;
        }
        label(r, text, s, msg, cx, sy + l.statusH * 0.6f, SDL_FColor{1, 0.6f, 0.5f, 0.95f}, 2);
    } else {
        label(r, text, size * 0.85f, lang_.get("launcher.note"), cx, sy + l.statusH * 0.6f, SDL_FColor{1, 1, 1, 0.5f}, 2);
    }
}

} // namespace engine
