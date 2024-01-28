#ifndef _MODENA_LUTE_TUNE_H_
#define _MODENA_LUTE_TUNE_H_

#include "geometry.h"
#include "dat_defs.h"
#include "hamming.h"

#include <iostream>

// Lute tunes, or LUT tuning.

namespace loc {

const char* fort_letters(unsigned c_index);

} // namespace loc

namespace mod {

using Margins = std::map<std::ptrdiff_t, std::string>;

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

    void sharpen(Slicer& slicer, Margins& margins) const;
    void inform(unsigned alphaid, unsigned portaid, unsigned c_index, const Point& fort, const Point& port, const Point& bay, const Point& air);
    void writeDirect(std::iostream& os) const;
};

void old_tune(Leftovers& lovers); // FIXME invert control and move to Leftovers

} // namespace mod

#endif

