#include "military.h"

#include "dat_defs.h"
#include "aynrand.h"

#include <vector>
#include <array>

namespace mil {

// Naive estimates on a small selection:
// Continent 1: 105 lines (35x3)
// A: HW=17 Fairies=16 Scare=17 Woods=14 Gob=15 Lun=8 Minesf=9 ... Can=2 FB=2 Bat=1 Sp=1
// D: HW=14 ... Bed=0-3
// Continent 2: 105 lines
// A: Wood Min Lun Fir Gob Can [Bed Cann Scar Spec]
// Continent 3: up to +50% headcount (random)
// Bat+Bed+Fir+Kob+Tit Spec Hun
// Continent 4: ditto. Dice: ((1..6)-2)*0.2
// Ill+Sal+Jinns+Wiz+Bat Hun Kob

//  A more robust historical distribution:
// { {Housewives, Fairies, Scarecrows, Goblins}, // 0
//   {Minesfolk, Woodsfolk, Canines,  Lunatics}, // 1
//   {Cannibals, Firstborn, Bedouins, Specters}, // 2
// },
// { {Minesfolk, Woodsfolk, Canines,  Lunatics}, // 1
//   {Cannibals, Firstborn, Bedouins, Specters}, // 2
//   {Housewives, Fairies, Scarecrows, Goblins}, // 0
// },
// { {Titans, Batmen, Kobolds, Huns}, // 3
//   {Cannibals, Firstborn, Bedouins, Specters}, // 2
//   {Jinns, Wizards, Illuminati, Salamanders}, // 4
// },
// { {Jinns, Wizards, Illuminati, Salamanders}, // 4
//   {Titans, Batmen, Kobolds, Huns}, // 3
//   {Cannibals, Firstborn, Bedouins, Specters}, // 2
// },

// Ratios: 8(main 4):4(next 4):2(rare 4):1(all other)

// Now a compact representation of the above:

using Tier = std::array<Unit, 4>;

std::vector<Tier> tiers = {
    {Housewives, Fairies, Scarecrows, Goblins}, // 0
    {Minesfolk, Woodsfolk, Canines,  Lunatics}, // 1
    {Cannibals, Firstborn, Bedouins, Specters}, // 2
    {Titans, Batmen, Kobolds, Huns}, // 3
    {Jinns, Wizards, Illuminati, Salamanders}, // 4
};

// struct Census {
//     unsigned main, next, rare; 
// };
enum Ordinarity {
    Ord_Main,
    Ord_Next,
    Ord_Rare,
    Ord_Anomalous
};
using Census = std::array<unsigned, Ord_Anomalous>;
std::vector<Census> censa = {{0, 1, 2}, {1, 2, 0}, {3, 2, 4}, {4, 3, 2}};

// FIXME ROADMAP these garrisons are semi-hardcoded in the original setup
// (there are or may be minor variations, e.g. 5024 housewives i/of 5000).
// We want to have a first playable ASAP, so we hardcode them for now.
// The eventual solution must allow tuning, saving and restoring garrison
// troop numbers as replayable missions. Particularly, such missions must
// stay replayable across map/terrain changes.

// P.S. May God bless Dina Talaat, for she is so beautiful and geniune,
//      and may the one who removes this comment be cursed.

std::vector<Army> regular = {
    // continent 1: 6 regular armies
    {{50, Housewives}, {20, Canines}, {25, Recruits}, {30, Housewives}, {30, Housewives}},
    {{10, Bedouins}, {30, Recruits}, {20, Recruits}, {60, Housewives}, {40, Housewives}},
    {{70, Fairies}, {50, Fairies}, {20, Scarecrows}, {20, Lunatics}, {4, Cannibals}},
    {{30, Canines}, {20, Goblins}, {10, Bowmen}, {2, Kobolds}, {6, Minesfolk}},
    {{50, Recruits}, {50, Recruits}, {10, Bowmen}, {10, Firstborn}, {5, Huns}},
    {{250, Fairies}, {10, Specters}, {10, Noblemen}, {10, Illuminati}, {10, Illuminati}},
    // continent 2: 4 regular armies
    {{100, Recruits}, {20, Bowmen}, {20, Spearmen}, {15, Horsemen}, {15, Noblemen}},
    {{30, Firstborn}, {30, Illuminati}, {10, Wizards}, {30, Spearmen}, {300, Fairies}},
    {{150, Goblins}, {20, Cannibals}, {10, Kobolds}, {5, Titans}, {80, Canines}},
    {{500, Scarecrows}, {100, Lunatics}, {30, Specters}, {10, Batmen}, {6, Jinns}},
    // continent 3: 4 regular armies
    {{600, Scarecrows}, {200, Lunatics}, {50, Specters}, {25, Batmen}, {10, Jinns}},
    {{30, Titans}, {5, Salamanders}, {30, Cannibals}, {200, Goblins}, {200, Woodsfolk}},
    {{300, Woodsfolk}, {40, Huns}, {20, Titans}, {100, Bedouins}, {700, Housewives}},
    {{35, Bowmen}, {100, Spearmen}, {80, Horsemen}, {60, Noblemen}, {5, Salamanders}},
    // continent 4: 3 regular armies
    {{30, Jinns}, {50, Batmen}, {100, Illuminati}, {500, Woodsfolk}, {5000, Housewives}},
    {{50, Jinns}, {10, Salamanders}, {200, Horsemen}, {250, Noblemen}, {60, Illuminati}},
    {{100, Salamanders}, {25, Salamanders}, {25, Salamanders}, {100, Jinns}, {100, Batmen}},
};

#define NO 0
#define MB 0
std::vector<UnitDef> udefs = {
    {  1, 1, {10, 20, MB,100}, "Housewives"},
    {  1, 1, {20, 50,100,127}, "Fairies"},
    {  2, 2, {NO, NO, NO, NO}, "Recruits"},
    {  3, 2, { 5, 15, 30, 75}, "Canines"},
    {  3, 2, { 5, 10, 25, MB}, "Scarecrows"},
    {  5, 2, { 5, 10, 25, 75}, "Lunatics"},
    {  5, 2, {10, 25, 50,100}, "Woodsfolk"},
    {  5, 2, { 5, 15, 30, 75}, "Goblins"},
    { 10, 2, {NO, NO, NO, NO}, "Bowmen"},
    { 10, 3, { 5, 10, 25, 50}, "Firstborn"},
    { 10, 3, {NO, NO, NO, NO}, "Spearmen"},
    { 15, 3, { 4,  8, 15, 30}, "Bedouins"},
    { 20, 3, { 4, 10, 20, 50}, "Minesfolk"},
    { 10, 4, { 2,  4, 10, 20}, "Specters"},
    { 35, 5, {NO, NO, NO, NO}, "Noblemen"},
    { 40, 4, { 2,  4,  8, 15}, "Cannibals"},
    { 40, 4, { 2,  4, 10, 20}, "Huns"},
    { 50, 4, { 2,  4,  8, 15}, "Kobolds"},
    { 20, 4, {NO, NO, NO, NO}, "Horsemen"},
    { 25, 5, { 2,  3,  6, 10}, "Wizards"},
    { 25, 5, { 2,  2,  4, 10}, "Illuminati"},
    { 30, 5, { 2,  4, 10, 25}, "Batmen"},
    { 60, 5, { 2,  2,  5, 10}, "Titans"},
    { 50, 6, { 2,  2,  5, 10}, "Jinns"},
    {200, 6, { 2,  2,  2,  5}, "Salamanders"},
};
#undef NO

const UnitDef& Stat(unsigned u) {
    return udefs.at(u);
}

Regiment Irregular(int continent) {
    // Remember, the ratio is 8:4:2:1
    unsigned bits = rnd::upto(0xfffff) >> 4;
    unsigned tune = 1 + ((bits >> 2) & 0xf);
    unsigned o_id = (tune & 0x8) ? 0 : (tune & 0x4) ? 1 : (tune & 0x2) ? 2 : 3;
    Regiment retval;
    if(o_id == Ord_Anomalous) {
        std::vector<Regiment> possibilities; // TODO extract and cache
        for(unsigned u = 0; u < udefs.size(); ++u) {
            const auto& udef = udefs.at(u);
            unsigned size = udef.occ.at(continent);
            if(udef.occ.at(continent)) {
                possibilities.push_back({size, (Unit)u});
            }
            retval = possibilities.at(rnd::upto(possibilities.size()));
        }
    } else {
        const auto& census = censa.at(continent);
        unsigned folk = bits & 0x3;
        retval.unit = tiers.at(census.at(o_id)).at(folk);
        retval.count = udefs.at(retval.unit).occ.at(continent);
    }
    // 50% bonus on senior continents (see estimates/inferences above)
    if(continent >= 2) {
        int dice = rnd::inrg(-1, 5);
        retval.count += std::trunc(0.2f * dice * retval.count);
    }
    return retval;
}

Army IrregularArmy(int continent, bool standing) {
    const unsigned size = (standing ? dat::kArmySlots : dat::kIdiotArmy);
    Army army;
    while(army.size() < size) {
        // NOTE we can have a more fair mix by stirring a pre-filled pool
        army.push_back(Irregular(continent));
    }
    return army;
}

Army Fort_Garrison(int continent, char lord) {
    switch(lord) {
        case dat::kCleansed:
            return {};
        case dat::kSquatter:
            return IrregularArmy(continent, true);
        default:
            return regular.at(lord); // copy out
    }
}

} // namespace mil