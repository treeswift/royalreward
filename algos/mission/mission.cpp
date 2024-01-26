#include "mission.h"

#include "dat_defs.h"
#include "legends.h"

#include "mumbling.h"

#include <cstdlib>
#include <cstdio>

namespace dat {

namespace {

// constexpr const char kCastles[kContinents] = {11, 6, 6, 3}; // total 26
// constexpr const char kEnemies[kContinents] =  {6, 4, 4, 3}; // total 17

constexpr unsigned kRanks = 4;

// TOOD All the text constants below should be moved to `legends.cpp` or
// TODO otherwise separated; `mission.cpp` should be reserved for logic,
// TODO not presentation.

const char* kCastes[][kRanks] = {
    {
        "Cadet",
        "Lieutenant",
        "Major",
        "Colonel"
    },
    {
        "Volunteer",
        "Activist",
        "Ambassador",
        "Champion"
    },
    {
        "Student",
        "Intern",
        "Master",
        "Doctor"
    },
    {
        "Athlete",
        "Participant",
        "Contender",
        "Medalist"
    },
};

} // anonymous

const char* Prototype::Name(int t, int rk) { return kCastes[t][rk]; }

Prototype::Prototype(Type t) {
    using namespace mil;
    switch (type = t)
    {
    case A:
        command = 100;
        army.push_back({20, Recruits});
        army.push_back({2, Bowmen});
        salary  = 1000;
        savings = 7500;
        is_a_bookworm = false;
        intuition = 1;
        education = 2;
        break;

    case B:
        command = 80;
        army.push_back({20, Housewives});
        army.push_back({20, Recruits});
        salary  = 1000;
        savings = 10000;
        is_a_bookworm = false;
        intuition = 1;
        education = 3;
        break;

    case C:
        command = 60;
        army.push_back({30, Housewives});
        army.push_back({10, Fairies});
        salary  = 3000;
        savings = 10000;
        is_a_bookworm = true;
        intuition = 2;
        education = 5;
        break;

    case D:
        command = 100;
        army.push_back({20, Canines});
        salary  = 2000;
        savings = 7500;
        is_a_bookworm = false;
        intuition = 0;
        education = 2;
        break;

    default:
        mum::bummer<std::out_of_range>("No such character class: %d\n", t);
    }
}

Mission::Mission() {
    allocTech();
}

void Mission::allocTech() {
    for(unsigned tech = 0; tech < kTechnologies; ++tech) {
        technologies.push_back(tech);
    }
    while(technologies.size() < kAlphabet) {
        technologies.push_back((char) rnd::upto(kTechnologies));
    }
    rnd::shuffle(technologies);
}

void Mission::chart(const map::Continent& cont) {
    chart(cont, loc::LordCount(continents()));
}

void Mission::propose(unsigned fortresses, unsigned enemies) {
    if(enemies > fortresses) {
        mum::bummer<std::logic_error>("Not enough fortresses (%u) to host the enemies (%u).\n",
            fortresses, enemies);
    }
    if(continents() >= kContinents) {
        mum::bummer<std::logic_error>("Four continents already defined.\n");
    }
    if(fortresses > free_forts) {
        mum::bummer<std::logic_error>("Defining too many forts/ports: %u out of %u.\n",
            fortresses, free_forts);
    }
    if(enemies > free_lords) {
        mum::bummer<std::logic_error>("Defining too many enemies: %u out of %u.\n",
            enemies, free_lords);
    }
    free_forts -= fortresses;
    free_lords -= enemies;
    if(continents() == kContinents - 1) {
        if(free_forts) {
            mum::bummer<std::logic_error>("Not all fortresses defined: %u left.\n", free_forts);
        }
        if(free_lords) {
            mum::bummer<std::logic_error>("Not all enemies defined: %u left.\n", free_lords);
        }
    }
}

// WISDOM: we extract SaveFile-independent metadata here
// to prevent lazy or read-time inferences/randomizations.
Intel::Intel(unsigned cidx, const map::Continent & cont) : lookback(cont) {
    using namespace map;
    // Indices within `valued_locs` are shuffled
    // by `specials()` in geo_wealth.cpp anyway.
    // Examine the map itself (or pre-extract tribe_locs).
    unsigned tunnel = 0;
    for(const Point& p : cont.wonder_locs) { // FIXME can be optimized by moving intel collection to `specials()`
        char c = at(cont.map, p);
        switch(c) {
            case cTribe: recruitment.push_back(mil::Recruiting(cidx));
                break;
            case cGift1: g1 = p;
                break;
            case cGift2: g2 = p;
                break;
            case cPaper: nn = p;
                break;
            case cGlass: mm = p;
                break;
            case cMetro: oo[tunnel++] = p;
                break;
            case cChest:
            case cAddMe:
                break;
            default:
                mum::bummer<std::out_of_range>("Unexpected object encountered on the map: %c at %d, %d. "
                                                "Human intervention required.\n", c, p.x, p.y);
        }
    }
    for(const Point& p : cont.enemy_locs) {
        (void) p; // can be accessed via lookback.enemy_locs
        rambling.push_back(mil::IrregularArmy(cidx, false));
    }
}

void Mission::chart(const map::Continent& cont, unsigned enemies) {
    unsigned fortresses = cont.forts_locs.size();
    unsigned curr_lords = kEnemies - free_lords;
    propose(fortresses, enemies);

    Nation nation = {continents(), curr_lords, 0};
    world.emplace_back(nation.continent, cont); // item constructor collects intel
        // on roaming armies. We may want refine the division of
        // responsibilities, here, but forts are special anyway,
        // and let's first make it work.
    while(nation.inner_idx < enemies) {
        geopolitics.push_back(nation);
        world.back().standing.push_back(mil::Fort_Garrison(nation.continent, nation.enemy_idx));
        ++nation.inner_idx;
        ++nation.enemy_idx;
    }
    nation.enemy_idx = kSquatter;
    while(nation.inner_idx < fortresses) {
        geopolitics.push_back(nation);
        world.back().standing.push_back(mil::IrregularArmy(nation.continent, true));
        ++nation.inner_idx;
    }
    gk.consider(cont);

    // seal the puzzle
    if(continents() == kContinents) {
        gk.select();
    }
}

} // namespace dat
