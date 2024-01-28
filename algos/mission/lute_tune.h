#ifndef _MODENA_LUTE_TUNE_H_
#define _MODENA_LUTE_TUNE_H_

#include "geometry.h"
#include "savefile.h" // FIXME invert dependency

#include <vector>

// Lute tunes, or LUT tuning.

namespace dat {

struct Port {
    map::Point loc;
    map::Shift bay;
    map::Shift air;
};

struct Fort {
    char cont;
    map::Point loc;
    Port port;
};

struct Land {
    Land();
    std::vector<Fort> forts;
};

const char* fort_letters(unsigned c_index);

void old_tune(dat::Leftovers& lovers); // FIXME invert control and move to Leftovers

} // namespace dat

#endif

