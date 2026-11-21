#include "engine/ui_draw.h"

#include <algorithm>
#include <cmath>

namespace engine::ui {

void fillRect(SDL_Renderer *r, float x, float y, float w, float h, SDL_FColor c)
{
    SDL_SetRenderDrawColorFloat(r, c.r, c.g, c.b, c.a);
    const SDL_FRect rect{x, y, w, h};
    SDL_RenderFillRect(r, &rect);
}

void frameRect(SDL_Renderer *r, float x, float y, float w, float h, float t, SDL_FColor c)
{
    fillRect(r, x, y, w, t, c);
    fillRect(r, x, y + h - t, w, t, c);
    fillRect(r, x, y + t, t, h - 2 * t, c);
    fillRect(r, x + w - t, y + t, t, h - 2 * t, c);
}

void disc(SDL_Renderer *r, float cx, float cy, float radius, SDL_FColor c)
{
    constexpr int kSegments = 40;
    SDL_Vertex v[kSegments + 2];
    int idx[kSegments * 3];
    v[0] = {{cx, cy}, c, {0, 0}};
    for (int i = 0; i <= kSegments; ++i) {
        const float a = static_cast<float>(i) / kSegments * 6.2831853f;
        v[i + 1] = {{cx + std::cos(a) * radius, cy + std::sin(a) * radius}, c, {0, 0}};
    }
    for (int i = 0; i < kSegments; ++i) {
        idx[i * 3] = 0;
        idx[i * 3 + 1] = i + 1;
        idx[i * 3 + 2] = i + 2;
    }
    SDL_RenderGeometry(r, nullptr, v, kSegments + 2, idx, kSegments * 3);
}

void ring(SDL_Renderer *r, float cx, float cy, float radius, float thickness, SDL_FColor c)
{
    constexpr int kSegments = 40;
    SDL_Vertex v[(kSegments + 1) * 2];
    int idx[kSegments * 6];
    for (int i = 0; i <= kSegments; ++i) {
        const float a = static_cast<float>(i) / kSegments * 6.2831853f;
        const float ca = std::cos(a), sa = std::sin(a);
        v[i * 2] = {{cx + ca * radius, cy + sa * radius}, c, {0, 0}};
        v[i * 2 + 1] = {{cx + ca * (radius - thickness), cy + sa * (radius - thickness)}, c, {0, 0}};
    }
    for (int i = 0; i < kSegments; ++i) {
        const int a = i * 2;
        const int k[6] = {a, a + 1, a + 2, a + 1, a + 3, a + 2};
        std::copy(k, k + 6, idx + i * 6);
    }
    SDL_RenderGeometry(r, nullptr, v, (kSegments + 1) * 2, idx, kSegments * 6);
}

void pill(SDL_Renderer *r, const SDL_FRect &rc, SDL_FColor c)
{
    constexpr int kArc = 20; // segments per round end
    const float rad = std::min(rc.h, rc.w) / 2;
    const float cy = rc.y + rc.h / 2;
    SDL_Vertex v[(kArc + 1) * 2 + 1];
    int idx[(kArc + 1) * 2 * 3];
    v[0] = {{rc.x + rc.w / 2, cy}, c, {0, 0}};
    int n = 1;
    for (int end = 0; end < 2; ++end) {
        const float cx = end == 0 ? rc.x + rc.w - rad : rc.x + rad;
        for (int i = 0; i <= kArc; ++i) {
            const float a = (static_cast<float>(i) / kArc + static_cast<float>(end) - 0.5f) * 3.14159265f;
            v[n++] = {{cx + std::cos(a) * rad, cy + std::sin(a) * rad}, c, {0, 0}};
        }
    }
    const int ring = n - 1;
    for (int i = 0; i < ring; ++i) {
        idx[i * 3] = 0;
        idx[i * 3 + 1] = 1 + i;
        idx[i * 3 + 2] = 1 + (i + 1) % ring;
    }
    SDL_RenderGeometry(r, nullptr, v, n, idx, ring * 3);
}

void panel(SDL_Renderer *r, float x, float y, float w, float h, float rowH)
{
    fillRect(r, x, y, w, h, kPanelFill);
    frameRect(r, x, y, w, h, std::max(2.0f, rowH * 0.04f), kPanelBorder);
}

} // namespace engine::ui
