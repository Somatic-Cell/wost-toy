#include "wost_toy/cli.hpp"
#include "wost_toy/domain_mixed_strip.hpp"
#include "wost_toy/solver.hpp"
#include "wost_toy/stats.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using Clock = std::chrono::high_resolution_clock;

namespace wost {

struct SummaryRow {
    int k = 0;
    Method method = Method::MC;
    SummaryStats stats;
    double exact = 0.0;
    double varianceRatioVsMC = 1.0;
    double mseRatioVsMC = 1.0;
    double timeRatioVsMC = 1.0;
    double workRatioVsMC = 1.0;
};

inline void ensure_parent_dir(const std::string& path) {
    const fs::path p(path);
    const auto parent = p.parent_path();
    if (!parent.empty()) fs::create_directories(parent);
}

inline void write_summary_csv(const std::string& path, const Config& cfg, const std::vector<SummaryRow>& rows) {
    ensure_parent_dir(path);
    std::ofstream out(path);
    if (!out) throw std::runtime_error("failed to open summary CSV: " + path);

    out << "k,method,N,M,eps,x0,y0,phase_bins,dist_bins,group_every,block_size,exact,mean,bias,std,variance,rmse,"
           "variance_ratio_vs_mc,mse_ratio_vs_mc,time_ms,time_ratio_vs_mc,work_ratio_vs_mc,mean_steps,mean_max_steps,mean_truncated\n";
    out << std::setprecision(17);
    for (const auto& r : rows) {
        out << r.k << ',' << to_string(r.method) << ','
            << cfg.N << ',' << cfg.M << ',' << cfg.eps << ',' << cfg.x0 << ',' << cfg.y0 << ','
            << cfg.phaseBins << ',' << cfg.distBins << ',' << cfg.groupEvery << ',' << cfg.blockSize << ','
            << r.exact << ',' << r.stats.mean << ',' << r.stats.bias << ',' << r.stats.stddev << ','
            << r.stats.variance << ',' << r.stats.rmse << ','
            << r.varianceRatioVsMC << ',' << r.mseRatioVsMC << ','
            << r.stats.timeMs << ',' << r.timeRatioVsMC << ',' << r.workRatioVsMC << ','
            << r.stats.meanSteps << ',' << r.stats.meanMaxSteps << ',' << r.stats.meanTruncated << '\n';
    }
}

inline void write_trial_header(std::ofstream& out) {
    out << "k,method,trial,estimate,exact,error,sq_error,mean_steps,max_steps_seen,truncated_count,time_ms\n";
}

inline SummaryRow run_method(const Config& cfg,
                             int k,
                             Method method,
                             std::ofstream* trialsOut) {
    MixedStripDomain domain(k);
    const double exact = domain.exact(cfg.x0, cfg.y0);

    std::vector<double> estimates;
    std::vector<double> meanSteps;
    std::vector<double> maxSteps;
    std::vector<double> truncated;
    estimates.reserve(static_cast<std::size_t>(cfg.M));
    meanSteps.reserve(static_cast<std::size_t>(cfg.M));
    maxSteps.reserve(static_cast<std::size_t>(cfg.M));
    truncated.reserve(static_cast<std::size_t>(cfg.M));

    const auto t0 = Clock::now();
    std::vector<double> perTrialTimeMs;
    perTrialTimeMs.reserve(static_cast<std::size_t>(cfg.M));

    for (int trial = 0; trial < cfg.M; ++trial) {
        const uint64_t methodSalt = static_cast<uint64_t>(method) * 0x9E3779B97F4A7C15ull;
        const uint64_t seed = cfg.seed ^ methodSalt ^ (static_cast<uint64_t>(k) * 0xBF58476D1CE4E5B9ull);

        const auto tt0 = Clock::now();
        const EstimateResult er = run_one_estimate(cfg, k, trial, method, seed);
        const auto tt1 = Clock::now();
        const double trialMs = std::chrono::duration<double, std::milli>(tt1 - tt0).count();
        perTrialTimeMs.push_back(trialMs);

        estimates.push_back(er.estimate);
        meanSteps.push_back(er.meanSteps);
        maxSteps.push_back(static_cast<double>(er.maxStepsSeen));
        truncated.push_back(static_cast<double>(er.truncatedCount));

        if (trialsOut) {
            const double err = er.estimate - exact;
            (*trialsOut) << k << ',' << to_string(method) << ',' << trial << ','
                         << std::setprecision(17) << er.estimate << ',' << exact << ',' << err << ',' << err * err << ','
                         << er.meanSteps << ',' << er.maxStepsSeen << ',' << er.truncatedCount << ',' << trialMs << '\n';
        }
    }
    const auto t1 = Clock::now();
    const double elapsedMs = std::chrono::duration<double, std::milli>(t1 - t0).count();

    SummaryRow row;
    row.k = k;
    row.method = method;
    row.exact = exact;
    row.stats = summarize(estimates, meanSteps, maxSteps, truncated, exact, elapsedMs);
    return row;
}

inline std::vector<SummaryRow> run_benchmark(const Config& cfg) {
    std::ofstream trialsOut;
    std::ofstream* trialsPtr = nullptr;
    if (!cfg.outTrials.empty()) {
        ensure_parent_dir(cfg.outTrials);
        trialsOut.open(cfg.outTrials);
        if (!trialsOut) throw std::runtime_error("failed to open trials CSV: " + cfg.outTrials);
        trialsOut << std::setprecision(17);
        write_trial_header(trialsOut);
        trialsPtr = &trialsOut;
    }

    std::vector<SummaryRow> allRows;

    for (int k : cfg.ks) {
        std::cout << "k=" << k << "\n";
        std::vector<SummaryRow> rowsForK;
        rowsForK.reserve(cfg.methods.size());

        for (Method m : cfg.methods) {
            std::cout << "  method=" << to_string(m) << " ... " << std::flush;
            SummaryRow row = run_method(cfg, k, m, trialsPtr);
            std::cout << "rmse=" << row.stats.rmse
                      << " std=" << row.stats.stddev
                      << " time_ms=" << row.stats.timeMs << "\n";
            rowsForK.push_back(row);
        }

        // Compute ratios against MC if present; otherwise against first method.
        const SummaryRow* base = nullptr;
        for (const auto& r : rowsForK) {
            if (r.method == Method::MC) { base = &r; break; }
        }
        if (!base && !rowsForK.empty()) base = &rowsForK.front();

        for (auto& r : rowsForK) {
            if (base) {
                r.varianceRatioVsMC = (base->stats.variance > 0.0) ? r.stats.variance / base->stats.variance : 0.0;
                r.mseRatioVsMC = (base->stats.rmse > 0.0) ? (r.stats.rmse * r.stats.rmse) / (base->stats.rmse * base->stats.rmse) : 0.0;
                r.timeRatioVsMC = (base->stats.timeMs > 0.0) ? r.stats.timeMs / base->stats.timeMs : 0.0;
                const double baseWork = base->stats.variance * base->stats.timeMs;
                const double work = r.stats.variance * r.stats.timeMs;
                r.workRatioVsMC = (baseWork > 0.0) ? work / baseWork : 0.0;
            }
            allRows.push_back(r);
        }
    }

    return allRows;
}

} // namespace wost

int main(int argc, char** argv) {
    try {
        wost::Config cfg = wost::parse_args(argc, argv);
        auto rows = wost::run_benchmark(cfg);
        wost::write_summary_csv(cfg.outSummary, cfg, rows);
        std::cout << "Wrote summary: " << cfg.outSummary << "\n";
        if (!cfg.outTrials.empty()) std::cout << "Wrote trials: " << cfg.outTrials << "\n";
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        wost::print_usage();
        return 1;
    }
    return 0;
}
