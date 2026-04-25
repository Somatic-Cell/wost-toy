#pragma once

#include <cmath>
#include <cstddef>
#include <numeric>
#include <vector>

namespace wost {

struct SummaryStats {
    double mean = 0.0;
    double bias = 0.0;
    double variance = 0.0;
    double stddev = 0.0;
    double rmse = 0.0;
    double meanSteps = 0.0;
    double meanMaxSteps = 0.0;
    double meanTruncated = 0.0;
    double timeMs = 0.0;
};

inline SummaryStats summarize(const std::vector<double>& estimates,
                              const std::vector<double>& meanSteps,
                              const std::vector<double>& maxSteps,
                              const std::vector<double>& truncated,
                              double exact,
                              double timeMs) {
    SummaryStats s;
    const std::size_t n = estimates.size();
    if (n == 0) return s;

    for (double x : estimates) s.mean += x;
    s.mean /= static_cast<double>(n);
    s.bias = s.mean - exact;

    double v = 0.0;
    double mse = 0.0;
    for (double x : estimates) {
        const double d = x - s.mean;
        v += d * d;
        const double e = x - exact;
        mse += e * e;
    }
    s.variance = (n > 1) ? v / static_cast<double>(n - 1) : 0.0;
    s.stddev = std::sqrt(s.variance);
    s.rmse = std::sqrt(mse / static_cast<double>(n));

    for (double x : meanSteps) s.meanSteps += x;
    for (double x : maxSteps) s.meanMaxSteps += x;
    for (double x : truncated) s.meanTruncated += x;
    s.meanSteps /= static_cast<double>(n);
    s.meanMaxSteps /= static_cast<double>(n);
    s.meanTruncated /= static_cast<double>(n);
    s.timeMs = timeMs;
    return s;
}

} // namespace wost
