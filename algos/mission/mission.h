#ifndef _ALGOS_MISSION_MISSION_H_
#define _ALGOS_MISSION_MISSION_H_

#include "military.h"

namespace dat {

// parameterize continents
    // write settings to/ read from an INI file
    // create an INI file with default settings
// randomize castles/enemies
// randomize cities /spells
// allocate troops to squatters
// allocate troops to followers
// allocate troops to dwellings
// instantiate character -- DONE
    // save character

struct Prototype {
    Prototype(const Prototype& other) = default;
    Prototype& operator=(const Prototype& other) = default;

    enum Type { A, B, C, D, Total };
    Prototype(Type t);

    static constexpr unsigned Count = Type::Total;
    static const char* Name(Type type);

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
