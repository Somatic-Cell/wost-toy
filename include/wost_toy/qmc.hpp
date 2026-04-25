#pragma once

#include "wost_toy/math.hpp"
#include <algorithm>
#include <cstdint>
#include <vector>

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
    // Includes i=0 -> 0.0. Good for complete dyadic blocks.
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

inline double pathwise_halton_sample(uint64_t walkerIndex, int step, double shift) {
    static const std::vector<uint32_t> primes = first_primes(4096);
    const uint32_t base = primes[static_cast<std::size_t>(step) % primes.size()];
    // +1 avoids the all-zero first point.
    return frac(radical_inverse(walkerIndex + 1, base) + shift);
}

struct Point2 {
    double x;
    double y;
};

inline Point2 rqmc_2d(uint64_t index, double shift0, double shift1) {
    // A simple 2D point set: first coord is base-2 VdC, second is irrational lattice.
    // This is intentionally dependency-free. For production, replace with OpenQMC/Sobol.
    constexpr double alpha = 0.75487766624669276005; // sqrt(2)-style irrational fractional constant
    return Point2{ frac(van_der_corput_base2(index) + shift0),
                   frac((static_cast<double>(index) + 0.5) * alpha + shift1) };
}

} // namespace wost
