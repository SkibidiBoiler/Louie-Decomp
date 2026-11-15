#pragma once

namespace engine {

// 2D affine transform, Flash convention: x' = a*x + c*y + tx, y' = b*x + d*y + ty.
struct Affine {
    float a = 1.0f, b = 0.0f, c = 0.0f, d = 1.0f, tx = 0.0f, ty = 0.0f;

    // Returns this * child: apply child first, then this.
    Affine operator*(const Affine &m) const
    {
        return {a * m.a + c * m.b,         b * m.a + d * m.b,
                a * m.c + c * m.d,         b * m.c + d * m.d,
                a * m.tx + c * m.ty + tx,  b * m.tx + d * m.ty + ty};
    }

    void apply(float x, float y, float &outX, float &outY) const
    {
        outX = a * x + c * y + tx;
        outY = b * x + d * y + ty;
    }

    static Affine translate(float x, float y) { return {1.0f, 0.0f, 0.0f, 1.0f, x, y}; }
    static Affine scale(float s) { return {s, 0.0f, 0.0f, s, 0.0f, 0.0f}; }
};

// Flash colour transform: out = in * mult + add (channels in 0..1).
struct ColorTransform {
    float mr = 1.0f, mg = 1.0f, mb = 1.0f, ma = 1.0f;
    float ar = 0.0f, ag = 0.0f, ab = 0.0f, aa = 0.0f;

    // Returns this * child: apply child first, then this.
    ColorTransform operator*(const ColorTransform &k) const
    {
        return {mr * k.mr, mg * k.mg, mb * k.mb, ma * k.ma,
                ar + mr * k.ar, ag + mg * k.ag, ab + mb * k.ab, aa + ma * k.aa};
    }
};

} // namespace engine
