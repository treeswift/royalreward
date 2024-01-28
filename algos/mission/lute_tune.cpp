#include "lute_tune.h"

#include "dat_defs.h"

#include <cstring>

// Lute tunes, or LUT tuning.

namespace dat {

constexpr const char conts[dat::kAlphabet] = {0, 1, 0, 1, 2, 0, 2, 2, 0, 2, 0, 2, 1,
                                              0, 0, 0, 1, 0, 3, 2, 3, 0, 0, 2, 1, 3};
constexpr const char fortx[dat::kAlphabet + 2] = "\x1e\x2f\x24\x1e\xb\x16\x29\x2b\xb\x29\x39\x34\x19"
                                                 "\x16\6\x3a\x2a\x36\x11\x09\x29\x28\x28\x2d\x13\x2e\x0b";
constexpr const char forty[dat::kAlphabet + 2] = "\x1b\6\x31\x12\x2e\x31\x24\x1b\x1e\x22\x3a\x39"
                                                 "\x27\x18\x39\x17\x38\6\x27\x12\x0c\5\x29\6\x13\x2b\7";

constexpr const char ptofs[dat::kAlphabet + 1] = "\x3\xe\x7\x10\xc\x16\x12\x15\x11\x13\xd\x5\x9\xf\xb\x2\x8\0\x6\x4\x1\x14\x18\xa\x17\x19";

const char* letters[4] = {
    // shuffling changes names of of specific<x,y> fort/port pair
    "VACFIKNOPRW", // keep V first to make H the first port ever
    "BDJMQY",
    "EGHLTX",
    "SUZ",
};

                                       //"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
const char mapping[dat::kAlphabet + 1] = "DOHQMWSVRTNFJPLCIAGEBUYKXZ";

Land::Land() : forts{} {
    ;
};

const char* fort_letters(unsigned c_index) {
    return letters[c_index];
}

void old_tune(dat::Leftovers& lovers) {
    constexpr unsigned kA = dat::kAlphabet;
    std::memcpy(lovers.conts, conts, kA);
    std::memcpy(lovers.forts[0], fortx, kA + 1);
    std::memcpy(lovers.forts[1], forty, kA + 1);
    // TODO havens, airfields, bays
    std::memcpy(lovers.ptofs, ptofs, kA);
}

} // namespace dat
