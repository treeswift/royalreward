#ifndef _MODENA_LUTE_TUNE_H_
#define _MODENA_LUTE_TUNE_H_

#include "geometry.h"
#include "dat_defs.h"

#include <iostream>

// Lute tunes, or LUT tuning.

namespace mod {

/**
 * Mission data that cannot be saved to SaveFile.
 */
struct Leftovers {
    enum Coordinate{X, Y, Dimensions};
    using Point = map::Point;

    char conts[dat::kAlphabet];
    char forts[Dimensions][dat::kAlphabet + 1];
    char ports[Dimensions][dat::kAlphabet];
    char p_bay[Dimensions][dat::kAlphabet];
    char p_air[Dimensions][dat::kAlphabet];
    char ptofs[dat::kAlphabet];

    void inform(unsigned alphaid, unsigned portaid, unsigned c_index, const Point& fort, const Point& port, const Point& bay, const Point& air);
    void writeDirect(std::ostream& os) const;
    void writeWisely(std::iostream& os) const { writeDirect(os); }; // only for now
};

void old_tune(Leftovers& lovers); // FIXME invert control and move to Leftovers

} // namespace mod

namespace loc {

const char* fort_letters(unsigned c_index);

} // namespace loc

#endif

