#pragma once

#include "wost_toy/config.hpp"
#include "wost_toy/math.hpp"
#include "wost_toy/rng.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <stdexcept>
#include <vector>

#ifdef WOST_TOY_USE_OPENQMC
#include <oqmc/sobol.h>
#endif

namespace wost {

inline uint64_t reverse_bits64(uint64_t x) {
    x = ((x & 0x5555555555555555ull) << 1) | ((x >> 1) & 0x5555555555555555ull);
    x = ((x & 0x3333333333333333ull) << 2) | ((x >> 2) & 0x3333333333333333ull);
    x = ((x & 0x0f0f0f0f0f0f0f0full) << 4) | ((x >> 4) & 0x0f0f0f0f0f0f0f0full);
    x = ((x & 0x00ff00ff00ff00ffull) << 8) | ((x >> 8) & 0x00ff00ff00ff00ffull);
    x = ((x & 0x0000ffff0000ffffull) << 16) | ((x >> 16) & 0x0000ffff0000ffffull);
    x = (x << 32) | (x >> 32);
    return x;
}

inline double van_der_corput_base2(uint64_t i) {
    const uint64_t r = reverse_bits64(i);
    return static_cast<double>(r >> 11) * (1.0 / 9007199254740992.0);
}

inline double rqmc_1d(uint64_t index, double shift) {
    return frac(van_der_corput_base2(index) + shift);
}

inline double radical_inverse(uint64_t index, uint32_t base) {
    double invBase = 1.0 / static_cast<double>(base);
    double invBi = invBase;
    double result = 0.0;
    while (index > 0) {
        const uint64_t digit = index % base;
        result += static_cast<double>(digit) * invBi;
        index /= base;
        invBi *= invBase;
    }
    return result;
}

inline std::vector<uint32_t> first_primes(std::size_t n) {
    std::vector<uint32_t> primes;
    primes.reserve(n);
    for (uint32_t x = 2; primes.size() < n; ++x) {
        bool ok = true;
        for (uint32_t p : primes) {
            if (static_cast<uint64_t>(p) * static_cast<uint64_t>(p) > x) break;
            if (x % p == 0) { ok = false; break; }
        }
        if (ok) primes.push_back(x);
    }
    return primes;
}

inline uint64_t hash_combine64(uint64_t x, uint64_t v) {
    x ^= v + 0x9E3779B97F4A7C15ull + (x << 6) + (x >> 2);
    return mix_u64(x);
}

inline uint64_t qmc_domain_key(uint64_t seed,
                               uint64_t tag,
                               uint64_t trial,
                               uint64_t step = 0,
                               uint64_t bucket = 0,
                               uint64_t extra = 0) {
    uint64_t x = seed;
    x = hash_combine64(x, tag);
    x = hash_combine64(x, trial);
    x = hash_combine64(x, step);
    x = hash_combine64(x, bucket);
    x = hash_combine64(x, extra);
    return x;
}

inline double pathwise_halton_sample(uint64_t walkerIndex, int step, double shift) {
    static const std::vector<uint32_t> primes = first_primes(4096);
    const uint32_t base = primes[static_cast<std::size_t>(step) % primes.size()];
    return frac(radical_inverse(walkerIndex + 1, base) + shift);
}

struct Point2 {
    double x;
    double y;
};

inline Point2 rqmc_2d(uint64_t index, double shift0, double shift1) {
    constexpr double alpha = 0.75487766624669276005;
    return Point2{ frac(van_der_corput_base2(index) + shift0),
                   frac((static_cast<double>(index) + 0.5) * alpha + shift1) };
}

struct QmcAddress {
    uint64_t domainKey = 0;
    uint64_t sampleIndex = 0;
    uint32_t dimension = 0;
};

class QmcEngine {
public:
    QmcEngine() = default;
    QmcEngine(QmcBackendKind kind, uint64_t seed) : kind_(kind), seed_(seed) {}

    QmcBackendKind kind() const { return kind_; }

    double sample1D(const QmcAddress& addr) const {
        switch (kind_) {
        case QmcBackendKind::Simple:
            return sample_simple_1d(addr);
        case QmcBackendKind::OpenQmcSobol:
            return sample_openqmc_sobol_1d(addr);
        }
        return sample_simple_1d(addr);
    }

    Point2 sample2D(const QmcAddress& addr) const {
        QmcAddress a = addr;
        a.dimension = 0;
        const double x = sample1D(a);
        a.dimension = 1;
        const double y = sample1D(a);
        return Point2{x, y};
    }

private:
    double sample_simple_1d(const QmcAddress& addr) const {
        static const std::vector<uint32_t> primes = first_primes(4096);
        const uint32_t base = primes[static_cast<std::size_t>(addr.dimension) % primes.size()];
        const double shift = hashed_unit(seed_, 0xC001u, addr.domainKey, addr.dimension);
        return frac(radical_inverse(addr.sampleIndex + 1, base) + shift);
    }

    double sample_openqmc_sobol_1d(const QmcAddress& addr) const {
#ifndef WOST_TOY_USE_OPENQMC
        (void)addr;
        throw std::runtime_error("OpenQMC backend was requested, but this binary was built without WOST_TOY_USE_OPENQMC=ON");
#else
        // OpenQMC domains draw at most 4 dimensions. For dimensions beyond 4,
        // branch into a child domain. This keeps each draw local to a high-quality
        // 4D Owen-scrambled Sobol pattern instead of depending on very high-dimensional projections.
        const uint32_t dimBlock = addr.dimension / 4u;
        const uint32_t localDim = addr.dimension % 4u;

        // Map the 64-bit domain key to OpenQMC's pixel/frame-style constructor.
        const uint64_t dk = hash_combine64(seed_, addr.domainKey);
        const int x = static_cast<int>(dk & 0x7fffffffull);
        const int y = static_cast<int>((dk >> 31) & 0x7fffffffull);
        const int frame = static_cast<int>((dk >> 16) & 0x7fffffffull);
        const int index = static_cast<int>(addr.sampleIndex & 0x7fffffffull);

        const oqmc::SobolSampler base(x, y, frame, index, nullptr);
        const oqmc::SobolSampler domain = (dimBlock == 0u)
            ? base
            : base.newDomain(static_cast<int>(dimBlock));

        float sample[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        domain.drawSample<4>(sample);
        return static_cast<double>(sample[localDim]);
#endif
    }

    QmcBackendKind kind_ = QmcBackendKind::Simple;
    uint64_t seed_ = 1;
};

} // namespace wost
