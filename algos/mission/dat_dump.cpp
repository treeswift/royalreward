#include "savefile.h"
#include "legends.h"

#include "cstring"
#include <iomanip>
#include <sstream>
#include <list>

namespace dat {

// debugging output

SavedLoc::operator std::string() const {
    std::stringstream s;
    s << (int) x << ", " << (int) y;
    return s.str();
}

SavedLoc::operator bool() const {
    return (x && x+1) || (y && y+1); // extract the idiom and use it explicitly
}

bool Aspects::within(const SavedLoc& loc) const {
    return window.covers({loc.x, loc.y});
}

// MOREINFO reuse?
struct Mark {
    Mark(std::stringstream& ss) : _ss(ss), _st_pos(ss.tellp()) {}
    ~Mark() { _ss.seekp(_st_pos); }
private:
    std::stringstream& _ss;
    std::streampos _st_pos;
};

namespace {

constexpr unsigned kUnits = static_cast<unsigned>(mil::Unit::Total); // remove "class" from "enum"?
struct Dist {
    unsigned freq[kUnits];
    unsigned maxc[kUnits];
    unsigned minc[kUnits];
    unsigned long sum[kUnits];

    Dist() {
        for(unsigned i = 0; i < kUnits; ++i) {
            minc[i] = ~(freq[i] = maxc[i] = 0);
        }
    }

    void onArmy(unsigned unit, unsigned count) {
        freq[unit]++;
        sum[unit]+= count;
        if(maxc[unit] < count) maxc[unit] = count;
        if(minc[unit] > count) minc[unit] = count;
    }

    void summarize(IO& out, unsigned c, unsigned i) const {
        auto name = mil::Name(i);
        Real avg = 1.f * sum[i] / freq[i];
        fprintf(&out, "c=%u freq=%5u mmax=%3u..%3u avg=%.4f\t# %s\n", c, freq[i], minc[i], maxc[i], avg, name);
    }

    void summarize(IO& out, unsigned c) const {
        for(unsigned i = 0; i < kUnits; ++i) {
            if(freq[i]) summarize(out, c, i);
        }
    }

};

Dist distrib[kContinents];

// TODO FIXME URGENT move out and make flexible
char contoffort[kAlphabet] = {0, 1, 0, 1, 2, 0, 2, 2, 0, 2, 0, 2, 1, 0, 0, 0, 1, 0, 3, 2, 3, 0, 0, 2, 1, 3, }; 

} // anonymous

void DumpStats(IO& out) {
    for(unsigned i = 0; i < kContinents; ++i) {
        fprintf(&out, "Continent: %d\n", i);
        distrib[i].summarize(out, i);
    }
}

void SaveFile::dump(IO& out, const Aspects& oo) const {
    // MOREINFO(mad): support XPath?
    auto pio = &out;
    std::stringstream prefix;
    const char* delim = " ";
#define TPRINTF(fmt, ...) fprintf(pio, fmt "\n", ##__VA_ARGS__)
#define GPRINTF(fmt, ...) TPRINTF("%s" fmt, prefix.str().c_str(), ##__VA_ARGS__)
#define HPRINTF(hdr) TPRINTF("%s", hdr)

    // Name: Value # Comment
    if(Aspects::Hero & oo.sections) {
        HPRINTF("\n## Character");
        std::string n{name, name + sizeof(name)};
        TPRINTF("Name: %s", n.c_str());
        TPRINTF("Type: %d # %s", type, Prototype::Name(type));
        TPRINTF("Rank: %d # %s", rank, Prototype::Name(type, rank));
    }

    // FIXME check for presence of per-continent sections here:
    // if(!(~Aspects::Hero & oo.sections)) return; 

    HPRINTF("\n## Continents");
    prefix = decltype(prefix){};
    for(unsigned i = oo.conts.f; i <= oo.conts.t; ++i) { // ...kContinents
        Mark mark(prefix);
        prefix << "c=" << i << delim;
        if(oo.sections & Aspects::Foes) {
            HPRINTF("# Followers");
            for(unsigned j = 0; j < map::kIdiots; ++j) {
                const SavedLoc& sl = idiots[i][j];
                if(sl) {
                    Mark mark(prefix);
                    prefix << "f=" << j << delim;
                    const std::string ipos = sl;
                    GPRINTF("x=%d y=%d\t# roaming army at %s", sl.x, sl.y, ipos.c_str());
                    for(unsigned k = 0; k < kIdiotArmy; ++k) {
                        byte size = itroops[i][j][k];
                        char unit = iunits[i][j][k];
                        if(unit + 1) {
                            distrib[i].onArmy(unit, size);
                            mil::UnitDef stat = mil::Stat(unit);
                            GPRINTF("u=%u sz=%u hp=%u sl=%u\t# %u %s HP=%u", unit, size, stat.hp, stat.sl, size, stat.name, stat.hp * size);
                        }
                    }
                }
            }
        }
        if(oo.sections & Aspects::Cell) {
            HPRINTF("# Recruitment");
            for(unsigned j = 0; j < map::kTribes; ++j) {
                const SavedLoc& sl = tribes[i][j];
                if(sl) { // tribes should never go away, but just in case...
                    Mark mark(prefix);
                    prefix << "t=" << j << delim;
                    const std::string tpos = sl;
                    char unit = trunits[i][j];
                    const char* name = mil::Name(unit);
                    unsigned size = trtroop[i][j];
                    GPRINTF("u=%u sz=%u\t# %u %s at %s", unit, size, size, name, tpos.c_str());
                }
            }
        }
    }

    if(oo.sections & Aspects::Fort) {
        HPRINTF("\n## Fortresses");
        prefix = decltype(prefix){};
        for(unsigned i = 0; i < kAlphabet; ++i) {
            Mark mark(prefix);
            char cont = contoffort[i];
            char lord = lords[i];
            prefix << "c=" << (int) cont << delim;
            prefix << "f=" << (char)('A' + i) << delim;
            prefix << "l=" << std::setw(2) << (0xff & lord) << std::setw(0) << delim;
            GPRINTF("\t# %s (%s), under %s", loc::FortName(i), loc::ContName(cont), loc::LordName(lord));
            for(unsigned j = 0; j < kArmySlots; ++j) {
                unsigned size = guards[i][j];
                char unit = gunits[i][j];
                if(unit + 1) {
                    mil::UnitDef stat = mil::Stat(unit);
                    GPRINTF("a=%d u=%d sz=%u hp=%u sl=%u\t# %u %s HP=%u", j, unit, size, stat.hp, stat.sl, size, stat.name, stat.hp * size);
                    if((lords[i] & 0x1f) == 0x1f) { // 0x7f or 0xff, or... what?
                        distrib[cont].onArmy(unit, size);
                    }
                }
            }
        }
    }
}

IO& operator<<(IO& out, const SaveFile& sf) {
    sf.dump(out);
    return out;
}

} // namespace dat 