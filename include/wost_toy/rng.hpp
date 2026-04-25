#pragma once

#include <cstdint>
#include <limits>

namespace wost {

// SplitMix64: compact, deterministic, good enough for seeding/random shifts.
// Public-domain style algorithm by Steele/Vigna; included here to avoid deps.
inline uint64_t splitmix64(uint64_t& x) {
    uint64_t z = (x += 0x9E3779B97F4A7C15ull);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}

inline uint64_t mix_u64(uint64_t x) {
    return splitmix64(x);
}

inline double u01_from_u64(uint64_t x) {
    // 53 random bits -> double in [0,1).
    return static_cast<double>(x >> 11) * (1.0 / 9007199254740992.0);
}

class Rng {
public:
    explicit Rng(uint64_t seed = 1) : state_(seed) {}

    uint64_t next_u64() { return splitmix64(state_); }
    double uniform01() { return u01_from_u64(next_u64()); }

private:
    uint64_t state_;
};

inline double hashed_unit(uint64_t seed, uint64_t a, uint64_t b = 0, uint64_t c = 0, uint64_t d = 0) {
    uint64_t x = seed;
    x ^= 0x9E3779B97F4A7C15ull + a + (x << 6) + (x >> 2);
    x ^= 0xBF58476D1CE4E5B9ull + b + (x << 6) + (x >> 2);
    x ^= 0x94D049BB133111EBull + c + (x << 6) + (x >> 2);
    x ^= 0xD2B74407B1CE6E93ull + d + (x << 6) + (x >> 2);
    return u01_from_u64(mix_u64(x));
}

} // namespace wost
