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
    SDL_Texture *content_ = nullptr, *mask_ = nullptr;
    int w_ = 0, h_ = 0;
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
    // removeChild for a clip added with addChild (destroys it).
    void removeChild(const MovieClip *clip);
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
    float x() const { return x_; }
    float y() const { return y_; }
    bool hasPosition() const { return hasPos_; }

    // Mouse events (flash.events.MouseEvent) registered by the game: CLICK, MOUSE_DOWN, ROLL_OVER.
    std::function<void()> onClick, onMouseDown, onRollOver;
    bool mouseEnabled = true;
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
    };
    std::vector<Attached> attached_;
    std::vector<std::pair<std::string, std::string>> texts_; // text field instance -> its text
    std::vector<std::pair<std::string, float>> alphas_;      // instance -> alpha set from code
    bool visible_ = true;
    int scriptDepth_ = 0;
    bool hasPos_ = false, hasScale_ = false;
    float x_ = 0, y_ = 0, scaleX_ = 1, scaleY_ = 1;
    bool isButton_ = false;
    uint64_t hoverTick_ = 0; // pointer tick when last under the pointer (buttons go back to "up" otherwise)
    bool hoverDown_ = false;
};

} // namespace engine
