#pragma once

#include "wost_toy/config.hpp"
#include "wost_toy/domain_mixed_strip.hpp"
#include "wost_toy/qmc.hpp"
#include "wost_toy/rng.hpp"
#include "wost_toy/state_keys.hpp"
#include "wost_toy/walker.hpp"
#include <algorithm>
#include <cstddef>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace wost {

class StepSampler {
public:
    virtual ~StepSampler() = default;
    virtual const char* name() const = 0;
    virtual void begin_trial(int trial, uint64_t seed, const Config& cfg) {
        trial_ = trial;
        seed_ = seed;
        (void)cfg;
    }
    virtual void assign(int step,
                        const MixedStripDomain& domain,
                        const Config& cfg,
                        std::vector<WalkerState>& walkers,
                        std::vector<double>& u_dir) = 0;

protected:
    int trial_ = 0;
    uint64_t seed_ = 0;
};

class MCSampler final : public StepSampler {
public:
    const char* name() const override { return "mc"; }
    void begin_trial(int trial, uint64_t seed, const Config& cfg) override {
        StepSampler::begin_trial(trial, seed, cfg);
        rng_ = Rng(seed ^ (0xA53A9E3779B97F4Aull + static_cast<uint64_t>(trial)));
    }
    void assign(int, const MixedStripDomain&, const Config&, std::vector<WalkerState>& walkers, std::vector<double>& u_dir) override {
        for (std::size_t i = 0; i < walkers.size(); ++i) {
            if (walkers[i].alive) u_dir[i] = rng_.uniform01();
        }
    }
private:
    Rng rng_{1};
};

class PathwiseRQMCSampler final : public StepSampler {
public:
    const char* name() const override { return "pathwise-rqmc"; }
    void assign(int step, const MixedStripDomain&, const Config&, std::vector<WalkerState>& walkers, std::vector<double>& u_dir) override {
        const double shift = hashed_unit(seed_, 0x1000u, static_cast<uint64_t>(trial_), static_cast<uint64_t>(step));
        for (std::size_t i = 0; i < walkers.size(); ++i) {
            if (walkers[i].alive) {
                // Fixed walker index drives the path dimension. This is a cheap pathwise RQMC baseline.
                u_dir[i] = pathwise_halton_sample(static_cast<uint64_t>(i), step, shift);
            }
        }
    }
};

class StepRQMCNoSortSampler final : public StepSampler {
public:
    const char* name() const override { return "step-rqmc-no-sort"; }
    void assign(int step, const MixedStripDomain&, const Config&, std::vector<WalkerState>& walkers, std::vector<double>& u_dir) override {
        const double shift = hashed_unit(seed_, 0x2000u, static_cast<uint64_t>(trial_), static_cast<uint64_t>(step));
        uint64_t local = 0;
        for (std::size_t i = 0; i < walkers.size(); ++i) {
            if (walkers[i].alive) {
                u_dir[i] = rqmc_1d(local++, shift);
            }
        }
    }
};

class FullSortArrayRQMCSampler final : public StepSampler {
public:
    const char* name() const override { return "full-sort-array-rqmc"; }
    void assign(int step, const MixedStripDomain& domain, const Config& cfg, std::vector<WalkerState>& walkers, std::vector<double>& u_dir) override {
        if (cfg.groupEvery > 1 && (step % cfg.groupEvery) != 0) {
            // Cheap fallback between grouping steps.
            const double shift = hashed_unit(seed_, 0x2F00u, static_cast<uint64_t>(trial_), static_cast<uint64_t>(step));
            for (std::size_t i = 0; i < walkers.size(); ++i) {
                if (walkers[i].alive) u_dir[i] = rqmc_1d(static_cast<uint64_t>(i), shift);
            }
            return;
        }

        active_.clear();
        for (std::size_t i = 0; i < walkers.size(); ++i) {
            if (walkers[i].alive) {
                walkers[i].sortKey = full_sort_key(walkers[i], domain);
                active_.push_back(static_cast<int>(i));
            }
        }
        std::sort(active_.begin(), active_.end(), [&](int a, int b) {
            return walkers[static_cast<std::size_t>(a)].sortKey < walkers[static_cast<std::size_t>(b)].sortKey;
        });

        const std::size_t n = active_.size();
        points_.resize(n);
        const double shift0 = hashed_unit(seed_, 0x3000u, static_cast<uint64_t>(trial_), static_cast<uint64_t>(step), 0);
        const double shift1 = hashed_unit(seed_, 0x3001u, static_cast<uint64_t>(trial_), static_cast<uint64_t>(step), 1);
        for (std::size_t j = 0; j < n; ++j) {
            points_[j] = rqmc_2d(static_cast<uint64_t>(j), shift0, shift1);
        }
        std::sort(points_.begin(), points_.end(), [](const Point2& a, const Point2& b) {
            return a.x < b.x;
        });
        for (std::size_t j = 0; j < n; ++j) {
            u_dir[static_cast<std::size_t>(active_[j])] = points_[j].y;
        }
    }
private:
    std::vector<int> active_;
    std::vector<Point2> points_;
};

class BucketedRQMCSampler final : public StepSampler {
public:
    explicit BucketedRQMCSampler(bool blockLocal = false) : blockLocal_(blockLocal) {}
    const char* name() const override { return blockLocal_ ? "block-bucketed-rqmc" : "bucketed-rqmc"; }

    void assign(int step, const MixedStripDomain& domain, const Config& cfg, std::vector<WalkerState>& walkers, std::vector<double>& u_dir) override {
        if (cfg.groupEvery > 1 && (step % cfg.groupEvery) != 0) {
            const double shift = hashed_unit(seed_, 0x3F00u, static_cast<uint64_t>(trial_), static_cast<uint64_t>(step));
            for (std::size_t i = 0; i < walkers.size(); ++i) {
                if (walkers[i].alive) u_dir[i] = rqmc_1d(static_cast<uint64_t>(i), shift);
            }
            return;
        }

        const int nb = num_buckets(cfg);
        if (!blockLocal_) {
            buckets_.assign(static_cast<std::size_t>(nb), {});
            for (std::size_t i = 0; i < walkers.size(); ++i) {
                if (!walkers[i].alive) continue;
                const uint32_t b = bucket_id(walkers[i], domain, cfg);
                walkers[i].bucketId = b;
                buckets_[b].push_back(static_cast<int>(i));
            }
            assign_buckets(step, cfg, buckets_, u_dir);
        } else {
            // CPU simulation of GPU block-local grouping: split walkers into blocks and bucket within block only.
            const int bs = std::max(1, cfg.blockSize);
            const int n = static_cast<int>(walkers.size());
            for (int begin = 0; begin < n; begin += bs) {
                const int end = std::min(n, begin + bs);
                buckets_.assign(static_cast<std::size_t>(nb), {});
                for (int i = begin; i < end; ++i) {
                    if (!walkers[static_cast<std::size_t>(i)].alive) continue;
                    const uint32_t b = bucket_id(walkers[static_cast<std::size_t>(i)], domain, cfg);
                    walkers[static_cast<std::size_t>(i)].bucketId = b;
                    buckets_[b].push_back(i);
                }
                assign_buckets(step, cfg, buckets_, u_dir, static_cast<uint64_t>(begin / bs));
            }
        }
    }

private:
    void assign_buckets(int step,
                        const Config& cfg,
                        const std::vector<std::vector<int>>& buckets,
                        std::vector<double>& u_dir,
                        uint64_t block = 0) {
        for (std::size_t b = 0; b < buckets.size(); ++b) {
            const auto& indices = buckets[b];
            if (indices.empty()) continue;
            const double shift = hashed_unit(seed_, 0x4000u + block, static_cast<uint64_t>(trial_),
                                             static_cast<uint64_t>(step), static_cast<uint64_t>(b));
            for (std::size_t j = 0; j < indices.size(); ++j) {
                u_dir[static_cast<std::size_t>(indices[j])] = rqmc_1d(static_cast<uint64_t>(j), shift);
            }
        }
    }

    bool blockLocal_ = false;
    std::vector<std::vector<int>> buckets_;
};

inline std::unique_ptr<StepSampler> make_sampler(Method method) {
    switch (method) {
    case Method::MC: return std::make_unique<MCSampler>();
    case Method::PathwiseRQMC: return std::make_unique<PathwiseRQMCSampler>();
    case Method::StepRQMCNoSort: return std::make_unique<StepRQMCNoSortSampler>();
    case Method::FullSortArrayRQMC: return std::make_unique<FullSortArrayRQMCSampler>();
    case Method::BucketedRQMC: return std::make_unique<BucketedRQMCSampler>(false);
    case Method::BlockBucketedRQMC: return std::make_unique<BucketedRQMCSampler>(true);
    }
    throw std::runtime_error("unknown sampler method");
}

} // namespace wost
