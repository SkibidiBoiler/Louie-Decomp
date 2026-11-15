#include "engine/movieclip.h"
#include "engine/textures.h"

#include <algorithm>

namespace engine {

namespace {

float clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }

void drawShape(const RenderContext &ctx, uint16_t id, const Affine &m, const ColorTransform &color)
{
    const ShapeBounds *bounds = ctx.symbols->shape(id);
    SDL_Texture *texture = ctx.textures->get(id);
    if (!bounds || !texture) {
        return;
    }
    float tw = 0.0f, th = 0.0f;
    SDL_GetTextureSize(texture, &tw, &th);
    const float scale = static_cast<float>(ctx.textures->scale());
    const float x0 = bounds->x, y0 = bounds->y;
    const float x1 = x0 + tw / scale, y1 = y0 + th / scale;

    // TODO: additive colour terms need a shader; only the multiplier is applied for now.
    const SDL_FColor tint{clamp01(color.mr), clamp01(color.mg), clamp01(color.mb), clamp01(color.ma)};
    SDL_Vertex v[4];
    const float corners[4][4] = {{x0, y0, 0, 0}, {x1, y0, 1, 0}, {x1, y1, 1, 1}, {x0, y1, 0, 1}};
    for (int i = 0; i < 4; ++i) {
        m.apply(corners[i][0], corners[i][1], v[i].position.x, v[i].position.y);
        v[i].color = tint;
        v[i].tex_coord = {corners[i][2], corners[i][3]};
    }
    static const int indices[6] = {0, 1, 2, 0, 2, 3};
    SDL_RenderGeometry(ctx.renderer, texture, v, 4, indices, 6);
}

} // namespace

MovieClip::MovieClip(const SymbolLibrary &symbols, const Timeline &timeline)
    : symbols_(&symbols), timeline_(&timeline)
{
    syncChildren(false);
}

void MovieClip::gotoFrame(int frame)
{
    frame_ = std::clamp(frame, 0, timeline_->frameCount - 1);
    syncChildren(false);
}

void MovieClip::advance()
{
    if (playing_ && timeline_->frameCount > 1) {
        frame_ = (frame_ + 1) % timeline_->frameCount;
    }
    syncChildren(true);
}

void MovieClip::syncChildren(bool advanceSurvivors)
{
    std::vector<Child> next;
    for (const Track &track : timeline_->tracks) {
        if (!track.aliveAt(frame_)) {
            continue;
        }
        auto it = std::find_if(children_.begin(), children_.end(),
                               [&](const Child &c) { return c.track == &track; });
        if (it != children_.end()) {
            if (advanceSurvivors && it->clip) {
                it->clip->advance();
            }
            next.push_back(std::move(*it));
            continue;
        }
        Child child;
        child.track = &track;
        if (symbols_->kind(track.charId) == CharKind::Sprite) {
            if (const Timeline *tl = symbols_->timeline(track.charId)) {
                child.clip = std::make_unique<MovieClip>(*symbols_, *tl);
            }
        }
        next.push_back(std::move(child));
    }
    std::stable_sort(next.begin(), next.end(),
                     [](const Child &a, const Child &b) { return a.track->depth < b.track->depth; });
    children_ = std::move(next);
}

void MovieClip::render(const RenderContext &ctx, const Affine &parent, const ColorTransform &parentColor) const
{
    for (const Child &child : children_) {
        const Track &track = *child.track;
        // TODO: masks (clip depth) need stencil clipping; for now mask shapes are skipped and
        // the masked content is drawn unclipped.
        if (track.clipDepth > 0) {
            continue;
        }
        const TimelineKey &key = track.keyAt(frame_);
        const Affine m = parent * key.matrix;
        const ColorTransform color = key.hasColor ? parentColor * key.color : parentColor;
        if (color.ma <= 0.0f && color.aa <= 0.0f) {
            continue;
        }
        if (child.clip) {
            child.clip->render(ctx, m, color);
        } else if (ctx.symbols->kind(track.charId) == CharKind::Shape) {
            drawShape(ctx, track.charId, m, color);
        }
        // TODO: morph shapes, static/edit text and buttons are not drawn yet.
    }
}

void MovieClip::accumulateBounds(const Affine &parent, float &minX, float &minY, float &maxX, float &maxY) const
{
    for (const Child &child : children_) {
        const Track &track = *child.track;
        if (track.clipDepth > 0) {
            continue;
        }
        const TimelineKey &key = track.keyAt(frame_);
        if (key.hasColor && key.color.ma <= 0.0f && key.color.aa <= 0.0f) {
            continue;
        }
        const Affine m = parent * key.matrix;
        if (child.clip) {
            child.clip->accumulateBounds(m, minX, minY, maxX, maxY);
        } else if (const ShapeBounds *b = symbols_->shape(track.charId)) {
            const float xs[2] = {b->x, b->x + b->w};
            const float ys[2] = {b->y, b->y + b->h};
            for (float x : xs) {
                for (float y : ys) {
                    float px = 0.0f, py = 0.0f;
                    m.apply(x, y, px, py);
                    minX = std::min(minX, px);
                    minY = std::min(minY, py);
                    maxX = std::max(maxX, px);
                    maxY = std::max(maxY, py);
                }
            }
        }
    }
}

} // namespace engine
