#ifndef _ALGOS_MISSION_MISSION_H_
#define _ALGOS_MISSION_MISSION_H_

#include "dat_defs.h"
#include "geography.h"
#include "military.h"

#include <vector>

namespace dat {

// parameterize continents
    // write settings to/ read from an INI file
    // create an INI file with default settings

// randomize castles/enemies
// randomize cities /spells
// allocate troops to squatters
// allocate troops to followers
// allocate troops to dwellings

/**
 * Fortress-haven pair.
 */
struct FHPair {
    // Fortress NAME-haven NAME correspondence is fixed in original resources;
    //  the rest is flexible.
    // We need just enough fortresses in a part of the world to sustain known
    // enemies, and we can, for a while, preserve the original fortress counts.
    // Haven -> technology correspondences can be shuffled independently.

    char continent;
    char inner_idx;
    char enemy_idx; // global
};

struct Mission {
    unsigned free_forts = kAlphabet;
    unsigned free_lords = kAlphabet;

    Mission(const Mission& other) = default;
    Mission& operator=(const Mission& other) = default;

    void chart(const map::Continent& cont, unsigned enemies);
    void chart(const map::Continent& cont); // default count

    std::vector<map::Continent*> world; // up to "parts"
    std::vector<FHPair> geopolitics; // up to "alphabet"
    std::string technologies;

    // castles to locations!
    // castles to enemies
    // castle troops
    // roaming troops
    // dwelling troops
    // map::Point fort, port;
    // map::Point airp, bayp;
};

struct Prototype {
    Prototype(const Prototype& other) = default;
    Prototype& operator=(const Prototype& other) = default;

    enum Type { A, B, C, D, Total };
    Prototype(Type t);

    static constexpr unsigned Count = Type::Total;
    static const char* Name(int type, int promotion = 0);

    Type type;

    // fight
    unsigned command;
    mil::Army army;
    // money
    unsigned savings;
    unsigned salary;
    // magic
    bool is_a_bookworm;
    unsigned intuition;
    unsigned education;

    inline const char* Name() const { return Name(type); }
};

} // namespace dat

#endif
