#include "savefile.h"

#include "memory.h"

namespace dat {

SavedLoc& SavedLoc::operator=(const map::Point& p) {
    x = p.x;
    y = p.y;
}

SaveFile::SaveFile() {
    *this = {}; // equivalent to memset(this, 0, sizeof(*this)) but respects RTTI
}

void SaveFile::setHeroLoc(const map::Point& p) {
    pos = p;
    last = p;
    avail[0] = true;
    map::screen(p).visit([this]WITH_XY {
        constexpr unsigned kPoW = 0;
        unsigned bit = (kPoW * map::kMapDim + y) * map::kMapDim + x;
        unsigned idx = bit >> 3;
        // stupid but foolproof
        visit[idx] |= 1 << (bit & 0x7);
    });
    mount = Mount::Land;
    boatPofW = kNada;
}

} // namespace dat