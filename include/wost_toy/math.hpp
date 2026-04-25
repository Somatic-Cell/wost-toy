#pragma once

#include <cmath>
#include <cstdint>
#include <limits>

namespace wost {

constexpr double kPi = 3.141592653589793238462643383279502884;

inline double frac(double x) {
    return x - std::floor(x);
}

inline double clamp(double x, double lo, double hi) {
    return std::max(lo, std::min(hi, x));
}

inline double fold_reflect01(double y) {
    // Reflects y into [0, 1] by mirror tiling: 0,1 are Neumann mirror planes.
    double t = std::fmod(y, 2.0);
    if (t < 0.0) t += 2.0;
    return (t <= 1.0) ? t : (2.0 - t);
}

} // namespace wost
