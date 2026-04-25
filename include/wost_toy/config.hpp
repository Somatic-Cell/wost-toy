#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace wost {

enum class Method {
    MC,
    PathwiseRQMC,
    StepRQMCNoSort,
    FullSortArrayRQMC,
    BucketedRQMC,
    BlockBucketedRQMC
};

inline std::string to_string(Method m) {
    switch (m) {
    case Method::MC: return "mc";
    case Method::PathwiseRQMC: return "pathwise-rqmc";
    case Method::StepRQMCNoSort: return "step-rqmc-no-sort";
    case Method::FullSortArrayRQMC: return "full-sort-array-rqmc";
    case Method::BucketedRQMC: return "bucketed-rqmc";
    case Method::BlockBucketedRQMC: return "block-bucketed-rqmc";
    }
    return "unknown";
}

inline bool parse_method(const std::string& s, Method& out) {
    if (s == "mc") { out = Method::MC; return true; }
    if (s == "pathwise" || s == "pathwise-rqmc") { out = Method::PathwiseRQMC; return true; }
    if (s == "step" || s == "step-rqmc-no-sort") { out = Method::StepRQMCNoSort; return true; }
    if (s == "full" || s == "full-sort" || s == "full-sort-array-rqmc") { out = Method::FullSortArrayRQMC; return true; }
    if (s == "bucket" || s == "bucketed" || s == "bucketed-rqmc") { out = Method::BucketedRQMC; return true; }
    if (s == "block-bucket" || s == "block-bucketed" || s == "block-bucketed-rqmc") { out = Method::BlockBucketedRQMC; return true; }
    return false;
}

struct Config {
    int N = 1024;                  // walkers per estimate
    int M = 128;                   // independent randomizations
    std::vector<int> ks{1, 4, 8, 16};
    std::vector<Method> methods{Method::MC, Method::PathwiseRQMC, Method::StepRQMCNoSort,
                                Method::FullSortArrayRQMC, Method::BucketedRQMC};

    double eps = 1e-4;
    int maxSteps = 10000;
    double x0 = 0.35;
    double y0 = 0.37;

    int phaseBins = 16;
    int distBins = 1;
    int groupEvery = 1;
    int blockSize = 256;

    uint64_t seed = 12345;
    std::string outSummary = "results/summary.csv";
    std::string outTrials = "";
};

} // namespace wost
