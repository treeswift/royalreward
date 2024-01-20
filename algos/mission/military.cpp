#include "military.h"

namespace mil {

const char* const unit_names[static_cast<unsigned>(Unit::Total)] = {
    "Housewives",
    "Fairies",
    "Recruits",
    "Canines",
    "Scarecrows",
    "Lunatics",
    "Woodsfolk",
    "Goblins",
    "Bowmen",
    "Firstborn",
    "Spearmen",
    "Bedouins",
    "Minesfolk",
    "Specters",
    "Noblemen",
    "Cannibals",
    "Huns",
    "Kobolds",
    "Horsemen",
    "Wizards",
    "Illuminati",
    "Batmen",
    "Titans",
    "Jinns",
    "Salamanders",
};

const char* unit_name(Unit u) {
    return unit_names[static_cast<unsigned>(u)];
}

} // namespace mil