#ifndef _ALGOS_MISSION_MILITARY_H_
#define _ALGOS_MISSION_MILITARY_H_

#include <vector>
#include <array>

namespace mil {

enum Unit {
    Housewives,
    Fairies,
    Recruits,
    Canines,
    Scarecrows,
    Lunatics,
    Woodsfolk,
    Goblins,
    Bowmen,
    Firstborn,
    Spearmen,
    Bedouins,
    Minesfolk,
    Specters,
    Noblemen,
    Cannibals,
    Huns,
    Kobolds,
    Horsemen,
    Wizards,
    Illuminati,
    Batmen,
    Titans,
    Jinns,
    Salamanders,
    Total
};

static_assert(25 == static_cast<int>(Unit::Total), "Exactly 5x5 troop types must be defined");

struct UnitDef {
    unsigned hp;
    unsigned sl;
    std::array<unsigned, 4> occ;
    const char* name;
};

const UnitDef& Stat(Unit u);

inline const char* Name(Unit u) { return Stat(u).name; }

struct Regiment {
    Unit unit;
    unsigned count;
};

using Army = std::vector<Regiment>;

} // namespace mil

#endif
