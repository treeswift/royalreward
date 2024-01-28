#ifndef _MODENA_LUTE_TUNE_H_
#define _MODENA_LUTE_TUNE_H_

#include "geometry.h"
#include "savefile.h" // FIXME invert dependency

// Lute tunes, or LUT tuning.

namespace loc {

const char* fort_letters(unsigned c_index);

void old_tune(dat::Leftovers& lovers); // FIXME invert control and move to Leftovers

} // namespace loc

#endif

