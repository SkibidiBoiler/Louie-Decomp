#include "engine/menu_bar.h"

#include <algorithm>
#include <cmath>

#include "engine/lang.h"
#include "engine/text.h"

namespace engine {

namespace {

constexpr float kStageWidth = 700, kStageHeight = 416;
constexpr float kSmallPiece = 16, kLargePiece = 23; // button piece widths
constexpr float kSmallSize = 12, kLargeSize = 24;   // label sizes
constexpr float kPadding = 7;
constexpr const char *kFont = "Arial Black";

SDL_FColor rgb(uint32_t c)
{
    return {static_cast<float>((c >> 16) & 0xFF) / 255.0f, static_cast<float>((c >> 8) & 0xFF) / 255.0f,
            static_cast<float>(c & 0xFF) / 255.0f, 1.0f};
}

std::unique_ptr<MovieClip> instance(const SymbolLibrary &symbols, const std::string &cls)
{
    const int id = symbols.findClass(cls);
    const Timeline *tl = id >= 0 ? symbols.timeline(static_cast<uint16_t>(id)) : nullptr;
    if (!tl) {
        return nullptr;
    }
    auto clip = std::make_unique<MovieClip>(symbols, *tl);
    clip->gotoLabel("up", false);
    clip->stop();
    return clip;
}

float clipHeight(const MovieClip &clip)
{
    float minX = 1e9f, minY = 1e9f, maxX = -1e9f, maxY = -1e9f;
    clip.accumulateBounds(Affine{}, minX, minY, maxX, maxY);
    return maxY > minY ? maxY - minY : 0;
}

} // namespace

std::unique_ptr<MovieClip> makeApiButton(const SymbolLibrary &symbols, TextLibrary *text, const std::string &label,
                                         bool large, float width)
{
    const float piece = large ? kLargePiece : kSmallPiece;
    const float size = large ? kLargeSize : kSmallSize;
    const float textWidth = text ? text->drawString(kFont, size, label, 0, 0, SDL_FColor{}, 0, Affine{}, false) : 0;
    float w = width > 0 ? width : textWidth + 4 + kPadding * 2;
    w = std::ceil(w / piece) * piece;
    auto button = MovieClip::createEmpty(symbols);
    const std::string set = large ? "api_largebutton_" : "api_smallbutton_";
    const int middles = std::max(0, static_cast<int>(w / piece) - 2);
    for (int i = 0; i < middles + 2; ++i) {
        const char *part = i == 0 ? "left" : (i == middles + 1 ? "right" : "middle");
        if (auto p = instance(symbols, set + part)) {
            p->setPosition(static_cast<float>(i) * piece, 0);
            p->mouseEnabled = false;
            p->mouseChildren = false;
            button->addChild(std::move(p), "");
        }
    }
    MovieClip::TextField field;
    field.font = kFont;
    field.size = size;
    field.color = rgb(0x6E3B0A);
    field.x = 0;
    field.y = 3;
    field.width = w;
    field.align = 2;
    field.text = label;
    button->addTextField("label", field);
    return button;
}

void MenuBar::setTitle(const Lang *lang, const std::string &titleKey)
{
    title_ = lang ? lang->get(titleKey) : titleKey;
}

void MenuBar::clear()
{
    active_ = false;
    top_.reset();
    bottom_.reset();
    buttons_.clear();
    pressed_ = -1;
}

void MenuBar::buildButton(Button &b)
{
    const std::string set = b.red ? "api_redbutton_" : (b.large ? "api_largebutton_" : "api_smallbutton_");
    const std::string over = b.large ? "api_largebutton_" : "api_smallbutton_";
    b.piece = b.large ? kLargePiece : kSmallPiece;
    const float textWidth = text_ ? text_->drawString(kFont, b.large ? kLargeSize : kSmallSize, b.label, 0, 0,
                                                      SDL_FColor{}, 0, Affine{}, false)
                                  : 0;
    const float wanted = textWidth + 4 + kPadding * 2;
    b.w = std::ceil(wanted / b.piece) * b.piece;
    const int middles = std::max(0, static_cast<int>(b.w / b.piece) - 2);
    for (int i = 0; i < middles + 2; ++i) {
        const char *part = i == 0 ? "left" : (i == middles + 1 ? "right" : "middle");
        if (auto p = instance(*symbols_, set + part)) {
            b.pieces.push_back(std::move(p));
        }
        if (auto o = instance(*symbols_, over + part + "_overlay")) {
            b.overlays.push_back(std::move(o));
        }
    }
    b.h = b.large ? 46.0f : 27.0f;
}

void MenuBar::setup(const SymbolLibrary &symbols, TextLibrary *text, const Lang *lang, const std::string &titleKey,
                    const std::vector<Item> &items, const Layout &layout)
{
    clear();
    symbols_ = &symbols;
    text_ = text;
    layout_ = layout;
    active_ = true;
    title_ = titleKey.empty() || !lang ? std::string() : lang->get(titleKey);
    if (layout.showTop) {
        top_ = instance(symbols, "api_menu_top_panel");
        topY_ = top_ ? -(clipHeight(*top_) - layout.topVisible) : 0;
    }
    if (layout.showBottom) {
        bottom_ = instance(symbols, "api_menu_bottom_panel");
        bottomY_ = kStageHeight - layout.bottomVisible;
    }
    for (const Item &item : items) {
        Button b;
        b.item = item;
        b.label = lang ? lang->get(item.labelKey) : item.labelKey;
        b.large = item.kind == Kind::Action;
        b.red = item.kind == Kind::Back;
        buildButton(b);
        buttons_.push_back(std::move(b));
    }
    // Back button at the top left, tabs in a centred row, the action button centred in the bottom panel.
    float tabsWidth = 0;
    for (const Button &b : buttons_) {
        if (b.item.kind == Kind::Tab) {
            tabsWidth += b.w + 4;
        }
    }
    float tabX = std::floor((kStageWidth - (tabsWidth - 4)) / 2);
    for (Button &b : buttons_) {
        if (b.item.kind == Kind::Back) {
            b.x = 12;
            b.y = 7;
        } else if (b.item.kind == Kind::Tab) {
            b.x = tabX;
            b.y = 8;
            tabX += b.w + 4;
        } else {
            b.x = std::floor((kStageWidth - b.w) / 2);
            b.y = kStageHeight - layout.bottomVisible + std::floor((layout.bottomVisible - b.h) / 2);
        }
    }
}

bool MenuBar::update(const PointerState &pointer, const std::function<void(const std::string &)> &onEvent)
{
    if (!active_) {
        return false;
    }
    int hit = -1;
    for (size_t i = 0; i < buttons_.size(); ++i) {
        Button &b = buttons_[i];
        b.over = enabled_ && pointer.x >= b.x && pointer.x < b.x + b.w && pointer.y >= b.y && pointer.y < b.y + b.h;
        b.down = b.over && pointer.down;
        if (b.over) {
            hit = static_cast<int>(i);
        }
    }
    if (pointer.pressed) {
        pressed_ = hit;
    }
    if (pointer.released) {
        const int was = pressed_;
        pressed_ = -1;
        if (hit >= 0 && hit == was && onEvent) {
            onEvent(buttons_[static_cast<size_t>(hit)].item.event); // may clear this bar
            return true;
        }
    }
    const bool overTop = layout_.showTop && pointer.y >= 0 && pointer.y < layout_.topVisible;
    const bool overBottom = layout_.showBottom && pointer.y >= kStageHeight - layout_.bottomVisible;
    return hit >= 0 || overTop || overBottom;
}

void MenuBar::render(const RenderContext &ctx) const
{
    if (!active_) {
        return;
    }
    if (top_) {
        top_->render(ctx, Affine::translate(0, topY_), ColorTransform{});
    }
    if (bottom_) {
        bottom_->render(ctx, Affine::translate(0, bottomY_), ColorTransform{});
    }
    const bool tabs = std::any_of(buttons_.begin(), buttons_.end(),
                                  [](const Button &b) { return b.item.kind == Kind::Tab; });
    if (text_ && !title_.empty() && !tabs) {
        text_->drawString(kFont, 24, title_, kStageWidth / 2, 32, rgb(0xAFAFAE), 2, Affine{});
    }
    for (const Button &b : buttons_) {
        const Affine at = Affine::translate(b.x, b.y + (b.down ? 1.0f : 0.0f));
        for (size_t i = 0; i < b.pieces.size(); ++i) {
            b.pieces[i]->render(ctx, at * Affine::translate(static_cast<float>(i) * b.piece, 0), ColorTransform{});
        }
        if (b.over) {
            ColorTransform half;
            half.ma = 0.5f;
            for (size_t i = 0; i < b.overlays.size(); ++i) {
                b.overlays[i]->render(ctx, at * Affine::translate(static_cast<float>(i) * b.piece, 0), half);
            }
        }
        if (text_) {
            const float size = b.large ? kLargeSize : kSmallSize;
            const SDL_FColor color = b.red ? rgb(0x972121) : rgb(0x6E3B0A);
            // The label field sits 3 px down; its baseline is about one ascent below that.
            text_->drawString(kFont, size, b.label, b.w / 2, 3 + size * 1.02f, color, 2, at);
        }
    }
}

} // namespace engine
