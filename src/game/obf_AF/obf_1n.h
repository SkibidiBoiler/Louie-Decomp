#pragma once
// AS: §_-AF§.§_-1n§ — static maths helpers. Ported so far: degreesToRads.
#include <numbers>

namespace obf_AF {

class obf_1n {
public:
    // AS: §_-1n§.degreesToRads
    static double degreesToRads(double degrees) { return degrees * std::numbers::pi / 180; }
};

} // namespace obf_AF
