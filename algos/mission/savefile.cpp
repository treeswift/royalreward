#include "savefile.h"

#include "memory.h"

namespace dat {

SavedLoc& SavedLoc::operator=(const map::Point& p) {
    x = p.x;
    y = p.y;
    return *this;
}

SaveFile::SaveFile() {
    memset(name, 0, sizeof(*this));
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

// debugging output

IO& operator<<(IO& out, const SaveFile& sf) {
    auto pio = &out;

    // Name: Value # Comment
    std::string name{sf.name, sf.name + sizeof(sf.name)};
    fprintf(pio, "Name: %s\n", name.c_str());
    fprintf(pio, "Type: %d # %s\n", sf.type, Prototype::Name(sf.type));
    fprintf(pio, "Rank: %d # %s\n", sf.rank, Prototype::Name(sf.type, sf.rank));

    // ....

    return out;
}

} // namespace dat