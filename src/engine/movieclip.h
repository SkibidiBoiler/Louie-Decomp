#pragma once
// Runtime instance of a timeline symbol: its own playhead plus persistent child instances.
#include <SDL3/SDL.h>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "engine/math.h"
#include "engine/symbols.h"

namespace engine {

class ShapeTextures;
class TextLibrary;

// Offscreen layers for masks (a track with a clip depth). Masks are drawn only when the render target is the
// window; inside an offscreen target (sheets, nested masks) the masked content is drawn unclipped.
class MaskLayers {
public:
    ~MaskLayers();
    // Renders `content` clipped by `mask`, both drawing in stage coordinates through the given transform.
    void draw(SDL_Renderer *renderer, const std::function<void(const Affine &)> &content,
              const std::function<void(const Affine &)> &mask);

private:
    struct Level {
        SDL_Texture *content = nullptr, *mask = nullptr;
        int w = 0, h = 0;
    };
    std::vector<Level> levels_; // one pair per nesting depth
    size_t depth_ = 0;
    int w_ = 0, h_ = 0, lw_ = 0, lh_ = 0;
};

struct RenderContext {
    SDL_Renderer *renderer = nullptr;
    const SymbolLibrary *symbols = nullptr;
    ShapeTextures *textures = nullptr;
    TextLibrary *text = nullptr; // static and edit texts; not drawn when null
    // Interpolation between the previous and the current logic step (0 = previous, 1 = current).
    bool interpolate = false;
    float alpha = 1.0f;
    MaskLayers *masks = nullptr; // masks are skipped (content drawn unclipped) when null
};

// The pointer (mouse or a finger) in stage coordinates, for one logic frame.
struct PointerState {
    float x = -1, y = -1;
    bool down = false;     // held
    bool pressed = false;  // went down this frame
    bool released = false; // went up this frame
};

class MovieClip {
public:
    MovieClip(const SymbolLibrary &symbols, const Timeline &timeline);
    ~MovieClip();
    MovieClip(const MovieClip &) = delete;
    MovieClip &operator=(const MovieClip &) = delete;
    // `new <Class>()` for a class linked to a symbol; nullptr when there is no such class (getDefinitionByName
    // throws in the original).
    static std::unique_ptr<MovieClip> create(const SymbolLibrary &symbols, std::string_view className);
    // `new MovieClip()`: an empty clip, for containers.
    static std::unique_ptr<MovieClip> createEmpty(const SymbolLibrary &symbols);

    int currentFrame() const { return frame_; }
    int frameCount() const { return timeline_->frameCount; }
    bool playing() const { return playing_; }
    void play() { playing_ = true; }
    void stop() { playing_ = false; }
    void gotoFrame(int frame);

    // AS3 semantics (1-based frames); the target frame's converted script runs, as in Flash.
    void gotoAndStop(int frame1) { goTo(frame1 - 1, false); }
    void gotoAndPlay(int frame1) { goTo(frame1 - 1, true); }
    bool gotoLabel(std::string_view label, bool play);
    // MovieClip.currentFrameLabel: the label on the current frame, or empty.
    std::string currentFrameLabel() const;

    const Timeline &timeline() const { return *timeline_; }
    bool visible() const { return visible_; }
    void setVisible(bool v) { visible_ = v; }

    // Timeline child sprite with this instance name (current frame), or nullptr.
    MovieClip *findChild(std::string_view name);
    // Transform of that child in this clip's space; false if there is no such child.
    bool childMatrix(std::string_view name, Affine &out) const;
    // Calls fn(instance name, clip) for every timeline sprite child of the current frame.
    void forEachChild(const std::function<void(const std::string &, MovieClip &)> &fn);
    // addChild: a runtime child drawn above the timeline children, with an identity transform.
    MovieClip &addChild(std::unique_ptr<MovieClip> clip, std::string name);
    // addChildAt(clip, 0): under everything already in this clip (the last one added this way is the lowest).
    MovieClip &addChildAtBottom(std::unique_ptr<MovieClip> clip, std::string name);
    // removeChild for a clip added with addChild. The clip is destroyed at the next collectGarbage (a screen
    // often removes the clip whose button handler is running).
    void removeChild(const MovieClip *clip);
    static void collectGarbage();
    // getChildAt(0) as a MovieClip; nullptr when that child is not a MovieClip (AS3 would throw).
    MovieClip *firstChildClip();
    // `<instance>.text` / `.htmlText` of a timeline text field child (HTML when it starts with '<').
    void setText(std::string_view instance, std::string text);
    // `<instance>.alpha` of a timeline child that is not a MovieClip (a text field, a shape).
    void setChildAlpha(std::string_view instance, float alpha);

    // One Flash frame: move the playhead (looping), then advance the children that survived.
    void advance();

    // DisplayObject.x / .y / .scaleX / .scaleY set from code: they replace the timeline's values for this
    // instance from then on (as in Flash). x() / y() are only meaningful once set.
    void setX(float v) { hasPos_ = true; x_ = v; }
    void setY(float v) { hasPos_ = true; y_ = v; }
    void setPosition(float x, float y) { hasPos_ = true; x_ = x; y_ = y; }
    void setScale(float sx, float sy) { hasScale_ = true; scaleX_ = sx; scaleY_ = sy; }
    void setRotation(float degrees) { rotation_ = degrees; } // DisplayObject.rotation (code-set, attached clips)
    float x() const { return x_; }
    float y() const { return y_; }
    bool hasPosition() const { return hasPos_; }

    // Mouse events (flash.events.MouseEvent) registered by the game: CLICK, MOUSE_DOWN, ROLL_OVER.
    std::function<void()> onClick, onMouseDown, onRollOver, onRollOut;
    bool mouseEnabled = true;
    bool mouseChildren = true; // false: the children take no mouse events (this clip may, with a handler)

    // A TextField created by code (`new TextField()` + addChild) in one of the SWF's fonts, drawn above the
    // children in this clip's space. The single line is aligned in [x, x + width] (align 0 left, 1 right,
    // 2 centre); y is the top of the line.
    struct TextField {
        std::string font = "Arial";
        float size = 12;
        SDL_FColor color{0, 0, 0, 1};
        float x = 0, y = 0, width = 100;
        int align = 0;
        std::string text;
        bool visible = true;
    };
    TextField &addTextField(const std::string &name, const TextField &field);
    TextField *textField(std::string_view name);

    // `.mask = <sibling>` set from code: this clip is drawn clipped by `mask`, a timeline child of the same
    // parent, which itself is no longer drawn. Null removes it.
    void setMask(MovieClip *mask);

    // A Bitmap added by code (`addChild(new Bitmap(data))`): part of a texture drawn at (x, y) with size w x h,
    // above the timeline children, in this clip's space.
    struct BitmapChild {
        SDL_Texture *texture = nullptr;
        SDL_FRect src{};
        float x = 0, y = 0, w = 0, h = 0;
    };
    void addBitmap(const BitmapChild &bitmap) { bitmaps_.push_back(bitmap); }
    void clearBitmaps() { bitmaps_.clear(); }
    bool isButton() const { return isButton_; } // a SimpleButton (DefineButton): frames up/over/down/hit
    // The topmost instance under (x, y) that takes mouse events: a button, or a clip with a handler.
    MovieClip *hitTest(const Affine &parent, float x, float y);
    // Moves the pointer for one logic frame: button states, then ROLL_OVER, MOUSE_DOWN and CLICK.
    static void dispatchPointer(MovieClip &root, const Affine &parent, const PointerState &pointer);

    void render(const RenderContext &ctx, const Affine &parent, const ColorTransform &parentColor) const;

    // Grows [minX,maxX]x[minY,maxY] by the visible shapes of the current frame, transformed by `parent`.
    void accumulateBounds(const Affine &parent, float &minX, float &minY, float &maxX, float &maxY) const;

private:
    struct Child {
        const Track *track = nullptr;
        std::unique_ptr<MovieClip> clip; // only for sprite children
    };

    void syncChildren(bool advanceSurvivors);
    Affine childLocal(const Child &child, bool interpolate, float alpha) const;
    void renderChild(const RenderContext &ctx, const Child &child, const Affine &parent,
                     const ColorTransform &parentColor) const;
    bool hasHandler() const { return onClick || onMouseDown || onRollOver; }
    // An attached clip's own transform: its code-set position, scale and rotation (identity otherwise).
    Affine ownLocal() const
    {
        const float sx = hasScale_ ? scaleX_ : 1.0f, sy = hasScale_ ? scaleY_ : 1.0f;
        const float tx = hasPos_ ? x_ : 0.0f, ty = hasPos_ ? y_ : 0.0f;
        if (rotation_ == 0.0f) {
            return Affine{sx, 0, 0, sy, tx, ty};
        }
        const float r = rotation_ * 3.14159265f / 180.0f, cs = SDL_cosf(r), sn = SDL_sinf(r);
        return Affine{sx * cs, sx * sn, -sy * sn, sy * cs, tx, ty};
    }
    void goTo(int frame, bool play);
    void runScript(int depth);

    const SymbolLibrary *symbols_;
    const Timeline *timeline_;
    int frame_ = 0;
    int prevFrame_ = -1; // frame shown one logic step ago, -1 after a jump/loop or on creation
    bool playing_ = true;
    std::vector<Child> children_; // sorted by depth
    struct Attached {
        std::string name;
        std::unique_ptr<MovieClip> clip;
        bool below = false; // added with addChildAt(..., 0): drawn under the timeline children
    };
    std::vector<Attached> attached_;
    std::vector<std::pair<std::string, std::string>> texts_; // text field instance -> its text
    std::vector<std::pair<std::string, float>> alphas_;      // instance -> alpha set from code
    std::vector<std::pair<std::string, TextField>> fields_;  // text fields made by code
    std::vector<BitmapChild> bitmaps_;                        // bitmaps added by code
    MovieClip *maskedBy_ = nullptr; // a sibling set as this clip's mask
    MovieClip *masking_ = nullptr;  // the sibling this clip masks (then it is not drawn itself)
    static MovieClip *lastOver_, *pressedOn_; // the pointer's targets (cleared when they are destroyed)
    static std::vector<std::unique_ptr<MovieClip>> graveyard_;
    bool visible_ = true;
    int scriptDepth_ = 0;
    bool hasPos_ = false, hasScale_ = false;
    float x_ = 0, y_ = 0, scaleX_ = 1, scaleY_ = 1, rotation_ = 0;
    bool isButton_ = false;
    uint64_t hoverTick_ = 0; // pointer tick when last under the pointer (buttons go back to "up" otherwise)
    bool hoverDown_ = false;
};

} // namespace engine
