#pragma once
// AS: §_-AF§.§_-AJ§ — easing curves (t, start, change, duration), like Robert Penner's. Ported so far: the two
// quadratic ones.
namespace obf_AF {

class obf_AJ {
public:
    // AS: §_-AJ§.§_-7d§ — quadratic ease-in.
    static double obf_7d(double t, double b, double c, double d)
    {
        t /= d;
        return c * t * t + b;
    }
    // AS: §_-AJ§.§_-5w§ — quadratic ease-out.
    static double obf_5w(double t, double b, double c, double d)
    {
        t /= d;
        return -c * t * (t - 2) + b;
    }
};

} // namespace obf_AF
