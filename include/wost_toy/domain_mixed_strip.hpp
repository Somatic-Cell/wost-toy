#pragma once

#include "wost_toy/config.hpp"
#include "wost_toy/math.hpp"
#include <cmath>

namespace wost {

class MixedStripDomain {
public:
    explicit MixedStripDomain(int k) : k_(k), a_(static_cast<double>(k) * kPi) {}

    double radius(double x) const {
        return std::min(x, 1.0 - x);
    }

    bool is_terminal(double x, const Config& cfg) const {
        return radius(x) <= cfg.eps;
    }

    double terminal_value(double x, double y_unfolded) const {
        if (x < 0.5) return 0.0;
        return right_boundary_value(y_unfolded);
    }

    double right_boundary_value(double y_unfolded) const {
        const double yr = fold_reflect01(y_unfolded);
        return std::cos(a_ * yr);
    }

    double exact(double x, double y_unfolded) const {
        const double yr = fold_reflect01(y_unfolded);
        // Stable form of sinh(a*x)/sinh(a) for moderate/high a.
        const double numerator = 1.0 - std::exp(-2.0 * a_ * x);
        const double denominator = 1.0 - std::exp(-2.0 * a_);
        const double ratio = std::exp(-a_ * (1.0 - x)) * numerator / denominator;
        return ratio * std::cos(a_ * yr);
    }

    double phase(double y_unfolded) const {
        const double yr = fold_reflect01(y_unfolded);
        double phi = std::fmod(static_cast<double>(k_) * yr, 2.0);
        if (phi < 0.0) phi += 2.0;
        return 0.5 * phi; // [0, 1)
    }

    int k() const { return k_; }

private:
    int k_;
    double a_;
};

} // namespace wost
