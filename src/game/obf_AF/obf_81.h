#pragma once
// AS: §_-AF§.§_-81§ — number and time formatting for the interface. Ported so far: §_-7w§ and §_-2i§.
#include <cmath>
#include <string>

#include "game/as3.h"

namespace obf_AF {

class obf_81 {
public:
    // AS: §_-81§.§_-7w§ — thousands separated by commas ("99,999").
    static std::string obf_7w(double value)
    {
        const std::string digits = as3::String(value);
        std::string out;
        for (int i = static_cast<int>(digits.size()) - 1; i >= 0; --i) {
            const int fromEnd = static_cast<int>(digits.size()) - 1 - i;
            if (fromEnd % 3 == 0 && fromEnd > 0 && digits[static_cast<size_t>(i)] != '-') {
                out = "," + out;
            }
            out = digits[static_cast<size_t>(i)] + out;
        }
        return out;
    }

    // AS: §_-81§.§_-2i§ — logic frames (30 a second) as "mm:ss" plus ":cc" hundredths, and "hh:" first.
    static std::string obf_2i(double frames, bool hundredths = true, bool hours = false)
    {
        double rest = frames;
        std::string out;
        double h = 0;
        if (hours) {
            h = std::floor(frames / 30 / 60 / 60);
            rest -= h * 30 * 60 * 60;
        }
        const double m = std::floor(rest / 30 / 60);
        rest -= m * 30 * 60;
        const double s = std::floor(rest / 30);
        rest -= s * 30;
        const double c = std::round(rest / 30 * 100);
        auto two = [](double v) { return (v < 10 ? "0" : "") + as3::String(v); };
        if (hours) {
            out += two(h) + ":";
        }
        out += two(m) + ":" + two(s);
        if (hundredths) {
            out += ":" + two(c);
        }
        return out;
    }
};

} // namespace obf_AF
