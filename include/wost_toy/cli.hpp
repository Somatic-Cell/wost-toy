#pragma once

#include "wost_toy/config.hpp"
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace wost {

inline std::vector<std::string> split_csv(const std::string& s) {
    std::vector<std::string> out;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, ',')) {
        if (!item.empty()) out.push_back(item);
    }
    return out;
}

inline std::vector<int> parse_int_list(const std::string& s) {
    std::vector<int> out;
    for (const auto& item : split_csv(s)) out.push_back(std::stoi(item));
    return out;
}

inline std::vector<Method> parse_method_list(const std::string& s) {
    std::vector<Method> out;
    for (const auto& item : split_csv(s)) {
        if (item == "all") {
            return {Method::MC, Method::PathwiseRQMC, Method::StepRQMCNoSort,
                    Method::FullSortArrayRQMC, Method::BucketedRQMC};
        }
        Method m;
        if (!parse_method(item, m)) {
            throw std::runtime_error("unknown method: " + item);
        }
        out.push_back(m);
    }
    return out;
}

inline void print_usage() {
    std::cout <<
R"(mixed_boundary_benchmark options:
  --out PATH                 summary CSV path [results/summary.csv]
  --out-trials PATH          optional per-trial CSV path
  --methods LIST             comma list: all,mc,pathwise,step,full,bucket,block-bucket [all]
  --ks LIST                  comma list of k values [1,4,8,16]
  --N INT                    walkers per estimate [1024]
  --M INT                    independent randomizations [128]
  --eps FLOAT                epsilon shell [1e-4]
  --max-steps INT            max steps before safety projection [10000]
  --x0 FLOAT                 query x [0.35]
  --y0 FLOAT                 query y [0.37]
  --phase-bins INT           bucketed phase bins [16]
  --dist-bins INT            bucketed distance bins [1]
  --group-every INT          group every k steps [1]
  --block-size INT           block size for block-bucketed [256]
  --seed UINT64              base seed [12345]
  --help                     show this help

Example:
  mixed_boundary_benchmark --methods all --ks 1,4,8,16 --N 1024 --M 128 --out results/summary.csv
)";
}

inline Config parse_args(int argc, char** argv) {
    Config cfg;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        auto need_value = [&](const std::string& name) -> std::string {
            if (i + 1 >= argc) throw std::runtime_error("missing value for " + name);
            return argv[++i];
        };
        if (arg == "--help" || arg == "-h") {
            print_usage();
            std::exit(0);
        } else if (arg == "--out") {
            cfg.outSummary = need_value(arg);
        } else if (arg == "--out-trials") {
            cfg.outTrials = need_value(arg);
        } else if (arg == "--methods") {
            cfg.methods = parse_method_list(need_value(arg));
        } else if (arg == "--ks") {
            cfg.ks = parse_int_list(need_value(arg));
        } else if (arg == "--N") {
            cfg.N = std::stoi(need_value(arg));
        } else if (arg == "--M") {
            cfg.M = std::stoi(need_value(arg));
        } else if (arg == "--eps") {
            cfg.eps = std::stod(need_value(arg));
        } else if (arg == "--max-steps") {
            cfg.maxSteps = std::stoi(need_value(arg));
        } else if (arg == "--x0") {
            cfg.x0 = std::stod(need_value(arg));
        } else if (arg == "--y0") {
            cfg.y0 = std::stod(need_value(arg));
        } else if (arg == "--phase-bins") {
            cfg.phaseBins = std::stoi(need_value(arg));
        } else if (arg == "--dist-bins") {
            cfg.distBins = std::stoi(need_value(arg));
        } else if (arg == "--group-every") {
            cfg.groupEvery = std::stoi(need_value(arg));
        } else if (arg == "--block-size") {
            cfg.blockSize = std::stoi(need_value(arg));
        } else if (arg == "--seed") {
            cfg.seed = static_cast<uint64_t>(std::stoull(need_value(arg)));
        } else {
            throw std::runtime_error("unknown option: " + arg);
        }
    }
    return cfg;
}

} // namespace wost
