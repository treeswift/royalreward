#include "savefile.h"
#include "mission.h"

#include <cstring>
#include <cstdlib>

namespace dat {

namespace {
using namespace map;

constexpr unsigned t1Half = 0x80;
constexpr char tInter = (char) t1Half; // interactive bit
constexpr char tWidth = 13;
// unique tiles
constexpr char tPlain = 0;
constexpr char tEntry = 1;
constexpr char tCCWLT = 2;
constexpr char tCCWRT = 6;
constexpr char tCCWLB = 3;
constexpr char tCCWRB = 7;
constexpr char tCRear = 4;
constexpr char tCGate = 5 | tInter;
constexpr char tHRaft = 8; // bordered raft
constexpr char tVRaft = 9; // no-border raft
constexpr char tHaven = 10 | tInter;
constexpr char tChest = 11 | tInter;
// plains, woods, hills, grave depend on unit type; `military.cpp` provides a lookup table
constexpr char tWagon = 12 | tInter;
constexpr char tTreeH = 13 | tInter;
constexpr char tHills = 14 | tInter;
constexpr char tGrave = 15 | tInter;
constexpr char tLabel = 16 | tInter;
constexpr char tEnemy = 17 | tInter;
constexpr char tGift1 = 18 | tInter;
constexpr char tGift2 = 19 | tInter;
// from now on, continuous tiles. 19+13 equals 32 which is ' ' and means -solid- open water
// seamed tiles (solid; edged are lower)
constexpr char tWater = 0x20;
constexpr char tWoods = tWater + tWidth;
constexpr char tSands = tWoods + tWidth;
constexpr char tRocks = tSands + tWidth;
// conversion error indicator
constexpr char tLapse = tRocks; // change to tWater in release

/**
 * Tile converter. Tied to the save file format, but implements
 * an open standard and therefore isn't really private. Extract
 * its class declaration into a header if you need it elsewhere.
 */

constexpr int lb = 1, rb = 2, lt = 4, rt = 8;
// 2   3
//   * 
// 0   1
constexpr char sqr[] = "\1\3\2\5\0\12\4\14\10";
// 6 7 8    4 c 8
// 3 4 5 => 5 0 a
// 0 1 2    1 3 2
constexpr char sea[] = "\0\3\0\5\0\12\0\14\0"; // ignore corners

struct TileConv {

    TileConv() {
        lut.resize(t1Half, tLapse); // ASCII-7
        lut[cPlain] = tPlain;
        lut[cEntry] = tEntry;
        lut[cCCWLB] = tCCWLB;
        lut[cCCWLT] = tCCWLT;
        lut[cCCWRB] = tCCWRB;
        lut[cCCWRT] = tCCWRT;
        lut[cCGate] = tCGate;
        for(char c = cCRear; c <= cCMaxF; ++c) {
            lut[c] = tCRear;
        }
        lut[cRafts] = tVRaft;
        lut[cHaven] = tHaven;
        lut[cChest] = tChest;
        lut[cPaper] = tChest;
        lut[cGlass] = tChest;
        lut[cMetro] = tHills;
        lut[cLabel] = tLabel;
        lut[cEnemy] = tEnemy;
        lut[cAddMe] = tEnemy; // too
        lut[cGift1] = tGift1;
        lut[cGift2] = tGift2;
        lut[cTribe] = tHills; // the wizard's cell stays this way

        // solid tiles
        lut[cWater] = tWater;
        lut[cWoods] = tWoods;
        lut[cSands] = tSands;
        lut[cRocks] = tRocks;

        cor_sea.resize(0x10, 0); // all 2^8 combinations, though only 13 are supported
        cor_sea[lt | rt] = -1; // ▀
        cor_sea[lb | rb] = -2; // ▄
        cor_sea[rt | rb] = -3; // ▐
        cor_sea[lt | lb] = -4; // ▌
        cor_sea[rt | rb | lb] = -5; // ▟
        cor_sea[lt | rt | rb] = -6; // ▜
        cor_sea[rb | lb | lt] = -7; // ▙
        cor_sea[rt | lt | lb] = -8; // ▛
        cor_sea[lt] = -9;  // ▘ // ok
        cor_sea[lb] = -12; // ▖ // 10 -> ?
        cor_sea[rt] = -10; // ▝ // 
        cor_sea[rb] = -11; // ▗
        cor_for = cor_sea;
        cor_for[lb] = -10; // ▖ // 10 -> ?
        cor_for[rt] = -11; // ▝ // 
        cor_for[rb] = -12; // ▗
        cor_snd = cor_mnt = cor_for;
    }

    char onetoone(char c) const {
        return lut.at(c);
    }

    int tileoff(char c, const ChrMap& map, const IntMap& seg, const Point& p) const {
        int msk = 0xf;
        int idx = 0;
        int chk = 0;
        (nearby(p) & bound(0, 64)).visit([&]WITH_XY {
            if(at(map, {x, y}) != c) {
                msk &= ~sqr[idx];
                chk |= 1 << idx;
            }
            ++idx;
        });
        if(!msk && cWater == c) {
            bool odd = (p.y ^ p.x) & 1;
            switch(chk) {
                case 0x084: msk = odd ? lt : rb; break;
                case 0x201: msk = odd ? lb : rt; break;
            }
        }
        switch(c) {
            case cSands: return cor_snd.at(msk);
            case cRocks: return cor_mnt.at(msk);
            case cWoods: return cor_for.at(msk);
        }
        return msk ? cor_sea.at(msk) : (tHRaft-tWater); // correction, not absolute
    }

    int tileoff(const ChrMap& map, const IntMap& seg, const Point& p) const {
        return tileoff(at(map, p), map, seg, p);
    }

    char tribe(mil::Landscape l) const {
        return tWagon + l; // and castle unit dwellings become signposts
    }

    char operator()(const ChrMap& map, const IntMap& seg, const Point& p) const {
        char c = at(map, p);
        char t = onetoone(c);
        if(c == cWater || c == cWoods || c == cSands || c == cRocks) {
            t += tileoff(c, map, seg, p); // always nonpositive
        }
        return t;
    }

    std::string lut;
    std::string cor_sea, cor_mnt, cor_for, cor_snd;

} tconv;

} // anonymous

SavedLoc& SavedLoc::operator=(const map::Point& p) {
    x = p.x;
    y = p.y;
    return *this;
}

SaveFile::SaveFile() {
    std::memset(name, 0, sizeof(*this));
}

std::string& SaveFile::sanitize(std::string& name, char rpl) {
    if(name.size() > kNameSize) {
        name.resize(kNameSize);
    }
    for(char & c : name) {
        if(c < ' ' || c == kSquatter) { // unprintable 7f
            c = rpl;
        }
    }
    if(name.empty()) {
        name = "Hank Morgan";
    }
    return name;
}

void SaveFile::setHeroName(std::string& nm) {
    std::memset(name, ' ', kNameSize);
    sanitize(nm);
    std::copy(nm.data(), nm.data() + nm.size(), name);
}

void SaveFile::setHeroType(Prototype::Type t) {
    Prototype p{t};
    type = p.type;
    // default rank is 0, i.e. ok
    intuition = p.intuition;
    education = p.education;
    smart = p.is_a_bookworm;
    base_command = curr_command = p.command;
    salary = p.salary;
    money = p.savings;
    // army
    unsigned slot = 0;
    for(; slot < p.army.size(); ++slot) {
        const auto& rg = p.army.at(slot);
        units[slot] = rg.unit;
        troops[slot] = rg.count;
    }
    for(; slot < kArmySlots; ++slot) {
        units[slot] = kNada;
        troops[slot] = 0;
    }
}

void SaveFile::setHeroLoc(const map::Point& p, unsigned continent) {
    pos = p;
    last = p;
    avail[continent] = true;
    map::screen(p).visit([this]WITH_XY {
        constexpr unsigned kPoW = 0;
        unsigned bit = (kPoW * map::kMapDim + y) * map::kMapDim + x + 1; // NOTE +1 is ad hoc
        unsigned idx = bit >> 3;
        // stupid but foolproof
        visit[idx] |= 1 << (bit & 0x7);
    });
    mount = Mount::Land;
    boatPofW = kNada;
}

void SaveFile::setUIOptions(const UIOptions& opt) {
    delay = opt.delay;
    sound = opt.sound;
    blimp = opt.wbeep;
    livid = opt.livid;
    sized = opt.sized;
    cgapal = opt.pal;
}

void SaveFile::setLevel(unsigned lvl) {
    switch(lvl) {
        case 0: d_left = 900;
            break;
        case 1: d_left = 600;
            break;
        case 2: d_left = 400;
            break;
        case 3: d_left = 200;
            break;
    // sadly, no way to harden it; we can set d_left to lower but the score multiplier is capped at 4
         default:
            fprintf(stderr, "Only levels supported are 0 to 3, passed: %u\n", level);
            abort();
    }
    level = lvl;
    steps_day = 40;
    days_week = 5;
}

void Leftovers::inform(unsigned alphaid, unsigned c_index, const Point& fort, const Point& port, const Point& bay, const Point& air) {
    conts[alphaid] = c_index;
#define LEFTOVER_ITERATE(from, to, dim, comp) \
        to[dim][alphaid] = from.comp;
#define LEFTOVER_SCATTER(from, to) \
        LEFTOVER_ITERATE(from, to, X, x) \
        LEFTOVER_ITERATE(from, to, Y, y)
    LEFTOVER_SCATTER(fort, forts);
    LEFTOVER_SCATTER(port, ports);
    LEFTOVER_SCATTER(bay, p_bay);
    LEFTOVER_SCATTER(air, p_air);
#undef LEFTOVER_SCATTER
#undef LEFTOVER_ITERATE
}

void Leftovers::writeDirect(std::ostream& os) const {
    constexpr int kGuide = Dimensions * kAlphabet;
    constexpr int kPolit = Dimensions * (kAlphabet + 1); // king
    os.seekp(0x1867d); os.write(conts, kAlphabet);
    // os.seekp(0x165cb); os.write(conts, kAlphabet);
    os.seekp(0x183f8); os.write(forts[0], kPolit);

    os.seekp(0x18481); os.write(ports[0], kGuide);
    os.seekp(0x1852d); os.write(p_bay[0], kGuide);
    os.seekp(0x18649); os.write(p_air[0], kGuide);
    // keep port-to-fort (0x18697) intact for now?
    //os.seekp(0x18697); // erasing this does not make A->A!
    //for(unsigned c=0; c<kAlphabet;) {os.put(c++);}

}

void SaveFile::setMission(const Mission& mission, Leftovers& lovers) {
    unsigned c_index = 0;
    unsigned natid = 0;
    SavedLoc nowhere = {kNada, kNada};
    SavedLoc gndzero = {0, 0};
    for(const auto & intel : mission.world) {
        const auto& cont = intel.lookback;
        setMap(c_index, cont); // ONLY manually adjust tribes AFTER THIS
        if(map::kMature != cont.kGround) {
            const Point& p = cont.rgate;
            // assert cCGate == at(cont.map, cont.rgate)
            // imply: lovers.conts[kAlphabet] = c_index;
            lovers.forts[Leftovers::X][kAlphabet] = p.x;
            lovers.forts[Leftovers::Y][kAlphabet] = p.y;
        }

        // copy over armies, treasures etc.
        // standing armies are spread over by continents, we may want to change that
        for(const auto& standing : intel.standing) {
            const Nation& nation = mission.geopolitics.at(natid);
            char alphaid = mission.toponymics.at(natid);
            lords[alphaid] = nation.enemy_idx;
            // DAT file spillovers
            unsigned inner = nation.inner_idx;
            lovers.inform(alphaid, nation.continent,
                cont.forts_locs.at(inner), cont.ports_locs.at(inner),
                cont.bay_points.at(inner), cont.air_fields.at(inner));
            // ... guards, gunits:
            unsigned slot_id = 0;
            for(const auto& reg : standing) {
                guards[alphaid][slot_id] = reg.count;
                gunits[alphaid][slot_id] = reg.unit;
                ++slot_id;
            }
            while(slot_id < kArmySlots) { // caution
                gunits[alphaid][slot_id++] = kNada;
            }
            ++natid;
        }

        // TODO extract the idiomatic pattern below
        unsigned idiot_id = 0;
        for(const auto& rambling : intel.rambling) {
            idiots[c_index][idiot_id] = cont.enemy_locs.at(idiot_id);
            unsigned slot_id = 0;
            for(const auto& reg : rambling) {
                iunits[c_index][idiot_id][slot_id] = reg.unit;
                itroops[c_index][idiot_id][slot_id] = reg.count;
                ++slot_id;
            }
            while(slot_id < kIdiotArmy) { // caution
                iunits[c_index][idiot_id][slot_id++] = kNada;
            }
            ++idiot_id;
        }
        while(idiot_id < map::kIdiots) {
            idiots[c_index][idiot_id] = gndzero; // sic
            ++idiot_id;
        }

        // TODO extract the idiomatic pattern below
        unsigned tribe_id = 0;
        for(const auto& tribe : intel.recruitment) {
            const Point& p = cont.tribe_locs.at(tribe_id);
            tribes[c_index][tribe_id] = p;
            trunits[c_index][tribe_id] = tribe.unit;
            trtroop[c_index][tribe_id] = tribe.count;
            maps[c_index][p.y][p.x] = tconv.tribe(mil::Stat(tribe.unit).l);
            ++tribe_id;
        }
        while(tribe_id < map::kTribes) {
            tribes[c_index][tribe_id] = gndzero;
            trunits[c_index][tribe_id] = kNada;
            trtroop[c_index][tribe_id] = 0;
            ++tribe_id;
        }

        // addmes
        unsigned addme_id = 0;
        for(const Point& p : cont.addme_locs) {
            addme[c_index][addme_id++] = p;
        }

        // special specials (xcpt trophies)
        tunnels[c_index][0] = intel.oo[0];
        tunnels[c_index][1] = intel.oo[1];
        allmaps[c_index] = intel.mm;
        if(c_index < kContinents - 1) {
            newmaps[c_index] = intel.nn;
        }
        ++c_index;
    }

    // extract into setTechs()...
    std::string techs = mission.technologies;
    techs.resize(kAlphabet, kTechControls + 0); // pad w/Raft
    std::copy(techs.data(), techs.data() + kAlphabet, techno);

    initWanted();
    setGoldenKey(mission.gk.selected());
}

// service methods called by setMission

void SaveFile::setMap(unsigned idx, const map::Continent& cont) {
    auto& out = maps[idx];
    map::Block area = map::bound(0, map::kMapDim);
    area.visit([&]WITH_XY {
        out[y][x] = tconv(cont.map, cont.seg, {x, y});
    });
    if(map::kMature != cont.kGround) {
        // the Golden Trail
        cont.trail.visit([&]WITH_XY {
            byte& c = out[y][x];
            if(!c) c = tVRaft;
        });
        // out[5][13] = tCGate; // dbg
    }
}

void SaveFile::initWanted() {
    wanted = kNada;
    unsigned o = 0;
    for(auto& offer : offers) {
        offer = o++;
    }
    best_offer = o--;
    last_offer = o;
}

void SaveFile::setGoldenKey(const map::GoldenKey::Burial spot) {
    key_ciph = rnd::inrg(0xf, 0xff);
    key_cont = key_ciph ^ spot.cindex;
    const map::Point& p = spot.p;
    keyl.x = key_ciph ^ p.x;
    keyl.y = key_ciph ^ p.y;
}


} // namespace dat