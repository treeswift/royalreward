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

const UnitDef& Stat(unsigned u);

inline const char* Name(unsigned u) { return Stat(u).name; }

struct Regiment {
    unsigned count;
    Unit unit;
};

using Army = std::vector<Regiment>;

Regiment Irregular(int continent);
Army IrregularArmy(int continent, bool standing);
Army Fort_Garrison(int continent, char lord);


} // namespace mil

#endif
