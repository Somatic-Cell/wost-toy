#pragma once

#include "wost_toy/config.hpp"
#include "wost_toy/domain_mixed_strip.hpp"
#include "wost_toy/math.hpp"
#include "wost_toy/samplers.hpp"
#include "wost_toy/walker.hpp"
#include <algorithm>
#include <numeric>
#include <vector>

namespace wost {

inline bool terminate_if_close(WalkerState& w, const MixedStripDomain& domain, const Config& cfg) {
    if (domain.is_terminal(w.x, cfg)) {
        w.value = domain.terminal_value(w.x, w.y);
        w.alive = false;
        return true;
    }
    return false;
}

inline void advance_walker(WalkerState& w, double u_dir, const MixedStripDomain& domain, const Config& cfg) {
    if (!w.alive) return;
    if (terminate_if_close(w, domain, cfg)) return;

    const double r = domain.radius(w.x);
    const double theta = 2.0 * kPi * u_dir;
    w.x += r * std::cos(theta);
    w.y += r * std::sin(theta);
    ++w.step;
}

inline EstimateResult run_one_estimate(const Config& cfg,
                                       int k,
                                       int trial,
                                       Method method,
                                       uint64_t methodSeed) {
    MixedStripDomain domain(k);
    auto sampler = make_sampler(method);
    sampler->begin_trial(trial, methodSeed, cfg);

    std::vector<WalkerState> walkers(static_cast<std::size_t>(cfg.N));
    for (auto& w : walkers) {
        w.x = cfg.x0;
        w.y = cfg.y0;
        w.alive = true;
        w.step = 0;
        w.value = 0.0;
    }

    std::vector<double> u_dir(static_cast<std::size_t>(cfg.N), 0.0);
    int truncated = 0;

    for (int step = 0; step < cfg.maxSteps; ++step) {
        bool anyAlive = false;
        for (const auto& w : walkers) {
            if (w.alive) { anyAlive = true; break; }
        }
        if (!anyAlive) break;

        std::fill(u_dir.begin(), u_dir.end(), 0.0);
        sampler->assign(step, domain, cfg, walkers, u_dir);

        for (std::size_t i = 0; i < walkers.size(); ++i) {
            if (walkers[i].alive) {
                advance_walker(walkers[i], u_dir[i], domain, cfg);
            }
        }
    }

    // Safety projection for walkers that exceeded maxSteps. Should be zero for sane eps/maxSteps.
    for (auto& w : walkers) {
        if (w.alive) {
            w.value = domain.terminal_value(w.x, w.y);
            w.alive = false;
            ++truncated;
        }
    }

    EstimateResult result;
    double sum = 0.0;
    double stepsSum = 0.0;
    int maxStepSeen = 0;
    for (const auto& w : walkers) {
        sum += w.value;
        stepsSum += static_cast<double>(w.step);
        maxStepSeen = std::max(maxStepSeen, w.step);
    }
    result.estimate = sum / static_cast<double>(cfg.N);
    result.meanSteps = stepsSum / static_cast<double>(cfg.N);
    result.maxStepsSeen = maxStepSeen;
    result.truncatedCount = truncated;
    return result;
}

} // namespace wost
