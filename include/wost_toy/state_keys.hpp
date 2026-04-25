#pragma once

#include "wost_toy/config.hpp"
#include "wost_toy/domain_mixed_strip.hpp"
#include "wost_toy/math.hpp"
#include "wost_toy/walker.hpp"
#include <algorithm>
#include <cstdint>

namespace wost {

inline int nearest_absorbing_side(const WalkerState& w) {
    return (w.x < 0.5) ? 0 : 1; // 0: left, 1: right
}

inline uint32_t bucket_id(const WalkerState& w, const MixedStripDomain& domain, const Config& cfg) {
    const int side = nearest_absorbing_side(w);
    const double ph = domain.phase(w.y);
    const int phaseBin = std::min(cfg.phaseBins - 1,
                                  std::max(0, static_cast<int>(std::floor(cfg.phaseBins * ph))));

    const double dist = std::min(w.x, 1.0 - w.x);
    int distBin = 0;
    if (cfg.distBins > 1) {
        const double dNorm = clamp(dist / 0.5, 0.0, 1.0 - 1e-12);
        distBin = std::min(cfg.distBins - 1,
                           std::max(0, static_cast<int>(std::floor(cfg.distBins * dNorm))));
    }

    return static_cast<uint32_t>(side + 2 * phaseBin + 2 * cfg.phaseBins * distBin);
}

inline int num_buckets(const Config& cfg) {
    return std::max(1, 2 * cfg.phaseBins * cfg.distBins);
}

inline double full_sort_key(const WalkerState& w, const MixedStripDomain& domain) {
    const double side = static_cast<double>(nearest_absorbing_side(w));
    const double ph = domain.phase(w.y);
    const double dist = std::min(w.x, 1.0 - w.x);
    // Side first, then boundary phase, then distance. Weights keep the ordering stable.
    return side + 0.90 * ph + 0.05 * clamp(dist / 0.5, 0.0, 1.0);
}

} // namespace wost
