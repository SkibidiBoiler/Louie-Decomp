#pragma once
// ActionScript 3 semantics the translated code relies on.
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <random>
#include <string>
#include <string_view>
#include <vector>

namespace as3 {

constexpr double NaN = std::numeric_limits<double>::quiet_NaN();

// Number(String): surrounding whitespace ignored, "" -> 0, anything unparsable -> NaN.
inline double Number(std::string_view s)
{
    size_t b = 0, e = s.size();
    while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) {
        ++b;
    }
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) {
        --e;
    }
    if (b == e) {
        return 0.0;
    }
    const std::string tmp(s.substr(b, e - b));
    char *end = nullptr;
    const double v = std::strtod(tmp.c_str(), &end);
    return (end && *end == '\0') ? v : NaN;
}

// String(Number) for the values the games use (integers, NaN, ordinary decimals).
inline std::string String(double v)
{
    if (std::isnan(v)) {
        return "NaN";
    }
    if (v == std::floor(v) && std::fabs(v) < 1e15) {
        return std::to_string(static_cast<long long>(v));
    }
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.15g", v);
    return buf;
}

// Math.random: uniform in [0, 1).
inline double random()
{
    static std::mt19937 rng{std::random_device{}()};
    return std::uniform_real_distribution<double>(0.0, 1.0)(rng);
}

// Math.round: halves round towards +infinity.
inline double round(double v) { return std::floor(v + 0.5); }

// String.split(sep) with a single-character separator: "" -> [""], like AS3.
inline std::vector<std::string> split(std::string_view s, char sep)
{
    std::vector<std::string> out;
    size_t start = 0;
    while (true) {
        const size_t pos = s.find(sep, start);
        out.emplace_back(s.substr(start, pos == std::string_view::npos ? std::string_view::npos : pos - start));
        if (pos == std::string_view::npos) {
            return out;
        }
        start = pos + 1;
    }
}

// A value from a mixed Array: the original keeps some fields as Number and others as String.
struct Value {
    bool isNumber = false;
    double number = 0.0;
    std::string string;

    static Value num(double v) { return {true, v, {}}; }
    static Value str(std::string v) { return {false, 0.0, std::move(v)}; }
    double asNumber() const { return isNumber ? number : Number(string); }
};

} // namespace as3
