#include "legends.h"

#include "dat_defs.h"

namespace loc {

namespace { 
using namespace dat;
} // anonymous

constexpr const char* forts[] = {
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

constexpr const char* ports[] = {
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

constexpr const char* techs[] = {
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

static_assert(sizeof(forts) == sizeof(void*) * kAlphabet, "Need one fort name per letter");
static_assert(sizeof(ports) == sizeof(void*) * kAlphabet, "Need one port name per letter");
static_assert(sizeof(techs) == sizeof(void*) * kTechnologies, "Need one tech name per context and control");

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

} // namespace loc