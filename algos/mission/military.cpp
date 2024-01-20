#include "military.h"

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

struct Census {
    unsigned main, next, rare; 
};
std::vector<Census> censa = {{0, 1, 2}, {1, 2, 0}, {3, 2, 4}, {4, 3, 2}};

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

} // namespace mil