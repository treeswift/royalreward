#include "legends.h"

#include "dat_defs.h"

namespace loc {

namespace { 
using namespace dat;
} // anonymous

constexpr const char* conts[kContinents] = {
    "Mediocria",
    "Redwoodia",
    "Caribbea",
    "Desertia",
};

constexpr const char* lords[kEnemies] = {
    // continent 1
    "Peter the Petty",
    "Ruddy the Hacker",
    "Hillary Clinton",
    "Jeffrey Epstein",
    "Abduwali Moose",
    "Alexander Dugin",
    // continent 2
    "Gen. William Sherman",
    "Prince Nimrond",
    "Polyphemus",
    "Otto Ostborn",
    // continent 3
    "Merrylane Mansion",
    "Eugene Wagner",
    "Kim Jong Hun",
    "Joseph Hide'em",
    // continent 4
    "John Maynard Keynes",
    "Claus Swab",
    "Chief Salamander",
};

constexpr const char* forts[kAlphabet] = {
    "Arkaim",
    "Brest",
    "Chenaux",
    "Dover",
    "East Babylon",
    "Fort Arthur",
    "Gibraltar",
    "Helsingor",
    "Ixtlan",
    "Juggernaut",
    "Kronstadt",
    "La Rochelle",
    "Maiden Tower",
    "Nestor's Palace",
    "Oktoberfest",
    "Platamon",
    "Qars",
    "Rolando",
    "Suomenlinna",
    "Tolukko",
    "Ungern",
    "Vineta",
    "Wolfsschanze",
    "Xasper",
    "Yehudah",
    "Zorge",
};

constexpr const char* ports[kAlphabet] = {
    "Aleppo",
    "Brighton Beach",
    "Casa Blanca",
    "Dubrovnik",
    "Eloquence",
    "Festivity",
    "Glory",
    "Heaven's Gate",
    "Ivy Glen",
    "Justice",
    "Kolhapur",
    "Loch Ness",
    "Mediolanum",
    "Nectaria",
    "Oslo",
    "Pearl Harbor",
    "Quo Vadis",
    "Regensburg",
    "Stella Maris",
    "Tel Fontainebleau",
    "Ursa Minor",
    "Venetia",
    "West Camp",
    "Xenophony",
    "Yurmala",
    "Zurbaghan",
};

constexpr const char* techs[kTechnologies] = {
    // battle, a-g
    "Breed",
    "Relocate",
    "Brimstone",
    "Wildfire",
    "Petrify",
    "Resuscitate",
    "Repentance",
    
    // travel, a-g
    "Raft",
    "Stasis",
    "SIGINT",
    "Fortress...",
    "Haven...",
    "Summon",
    "Rally",
};

const char* ContName(unsigned index) {
    return conts[index];
}

const char* FortName(unsigned index) {
    return forts[index];
}

const char* PortName(unsigned index) {
    return ports[index];
}

const char* TechName(unsigned index) {
    return techs[index];
}

const char* TechName(unsigned index, bool combat) {
    return TechName(index + dat::kTechControls * !combat);
}

const char* LordName(char code) {
    switch(code) {
        case kCleansed:
            return "your rule";
        case kSquatter:
            return "squatters";
        default:
           return lords[code & 0x1f];
    }
}

} // namespace loc