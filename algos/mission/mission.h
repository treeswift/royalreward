#ifndef _ALGOS_MISSION_MISSION_H_
#define _ALGOS_MISSION_MISSION_H_

#include "dat_defs.h"
#include "geography.h"
#include "goldenkey.h"
#include "military.h"

#include <vector>
#include <string>

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
struct Nation {
    // Fortress NAME-haven NAME correspondence is fixed in original resources;
    //  the rest is flexible.
    // We need just enough fortresses in a part of the world to sustain known
    // enemies, and we can, for a while, preserve the original fortress counts.
    // Haven -> technology correspondences can be shuffled independently.

    char continent;
    char enemy_idx; // global
    unsigned inner_idx;
};

struct Intel {
    const map::Continent & lookback;
    std::vector<mil::Army> standing;
    std::vector<mil::Army> rambling;
    std::vector<mil::Regiment> recruitment;
    map::Point mm, nn, g1, g2; // specials
    map::Point oo[2]; // tunnel

    Intel(const Intel&) = default;
    Intel& operator=(const Intel&) = default;
    Intel(unsigned cidx, const map::Continent& cont, mil::Wild& fortune);
};

struct Mission {
    unsigned free_forts = kAlphabet;
    unsigned free_lords = kEnemies;

    Mission(const Mission& other) = default;
    Mission& operator=(const Mission& other) = default;

    // ROADMAP inject Legends -- either here,
    // or later to keep the ctor lightweight.
    // For now, let Modena care about it.
    Mission(rnd::Ayn ayn = {});

    inline unsigned seed() const { return rnd.seed(); }
    inline unsigned continents() const { return world.size(); }

    // the continent map is updated by these calls
    void chart(map::Continent& cont, unsigned enemies);
    void chart(map::Continent& cont); // default count

    const char* fort_letters(unsigned c_index) const;
    char fort_letter(unsigned c_index, unsigned f_index) const;

    rnd::Ayn rnd;
    std::vector<Intel> world; // up to kContinents
    std::vector<Nation> geopolitics; // up to kAlphabet
    std::string technologies;
    mil::Wild fortune;
    std::array<std::string, kContinents> letters;
    map::GoldenKey gk;

private:
    void allocTech(rnd::Ayn rnd);
    void propose(unsigned fortresses, unsigned enemies);
    void allocLetters();
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
