#pragma once

#include <cstdint>

namespace wost {

struct WalkerState {
    double x = 0.0;
    double y = 0.0;          // unfolded coordinate; Neumann handled by reflection in boundary eval
    bool alive = true;
    int step = 0;
    double value = 0.0;
    uint32_t bucketId = 0;
    double sortKey = 0.0;
};

struct EstimateResult {
    double estimate = 0.0;
    double meanSteps = 0.0;
    int maxStepsSeen = 0;
    int truncatedCount = 0;
};

} // namespace wost
