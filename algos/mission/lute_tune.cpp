#include "lute_tune.h"

#include "dat_defs.h"

#include <cstring>

// Lute tunes, or LUT tuning.

namespace dat {

constexpr const char conts[dat::kAlphabet] = {0, 1, 0, 1, 2, 0, 2, 2, 0, 2, 0, 2, 1,
                                              0, 0, 0, 1, 0, 3, 2, 3, 0, 0, 2, 1, 3};
constexpr const char* fortx = "\x1e\x2f\x24\x1e\xb\x16\x29\x2b\xb\x29\x39\x34\x19"
                               "\x16\x6\x3a\x2a\x36\x11\x9\x29\x28\x28\x2d\x13\x2e\xb";
constexpr const char* forty = "\x1b\x6\x31\x12\x2e\x31\x24\x1b\x1e\x22\x3a\x39"
                               "\x27\x18\x39\x17\x38\x6\x27\x12\xc\x5\x29\x6\x13\x2b\x7";

void old_tune(dat::Leftovers& lovers) {
    constexpr unsigned kA = dat::kAlphabet;
    std::memcpy(lovers.conts, conts, kA);
    std::memcpy(lovers.forts[0], fortx, kA);
    std::memcpy(lovers.forts[1], forty, kA);
}

} // namespace dat
