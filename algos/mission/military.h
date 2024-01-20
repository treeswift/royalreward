#ifndef _ALGOS_MISSION_MILITARY_H_
#define _ALGOS_MISSION_MILITARY_H_

#include <vector>

namespace mil {

enum class Unit {
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

const char* unit_name(Unit u);

struct Regiment {
    Unit unit;
    unsigned count;
};

using Army = std::vector<Regiment>;

} // namespace mil

#endif
