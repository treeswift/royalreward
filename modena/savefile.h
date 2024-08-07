#ifndef _ALGOS_MISSION_SAVEFILE_H_
#define _ALGOS_MISSION_SAVEFILE_H_

#include "map_defs.h"
#include "dat_defs.h"
#include "maps.h"
#include "mission.h"
#include "geography.h"
#include "goldenkey.h"
#include "lute_tune.h"

#include <cstdint>
#include <iostream>

namespace dat {

constexpr unsigned kWantedSlots = 5;
constexpr unsigned kPackedMaps = (map::kMapMem * kContinents) >> 3;
constexpr unsigned kVolunteers = map::kAddMes;

struct UIOptions {
    unsigned delay = 4;
    bool sound = true;
    bool wbeep = true;
    bool livid = true;
    bool sized = true;
    unsigned pal = 0;
}; // TODO move to UI

// move to IOO, also extract from map::
using IO = decltype(*stdout);

using byte = unsigned char;

#pragma pack(1)
struct SavedLoc {
    char x, y;

    SavedLoc& operator=(const map::Point& p);
    operator std::string() const;
    operator bool() const;
};

struct Aspects {
    // query ranges are inclusive, for simplicity
    struct Rng { unsigned f, t; };
    enum Sec {
        Hero = 1 << 0,
        Fort = 1 << 1,
        Port = 1 << 2,
        View = 1 << 3,
        Foes = 1 << 4,
        Gift = 1 << 5,
        Stat = 1 << 6,
        Cell = 1 << 7,
        Find = 1 << 31,
        Everything = ~0
    };
    unsigned sections = Everything;

    Rng conts = {0, kContinents-1};
    Rng forts = {0, kAlphabet - 1};
    Rng ports = {0, kAlphabet - 1};
    map::Block window = map::bound(0, map::kMapDim);

    bool within(const SavedLoc& loc) const;
};

/**
 * The binary layout of the following data structure is based on a verbal description from ModdingWiki:
 *  https://moddingwiki.shikadi.net/wiki/King%27s_Bounty_Saved_game_Format
 *  https://moddingwiki.shikadi.net/wiki/King%27s_Bounty_Map_Format
 *
 * No copyrighted code (from OpenKB or otherwise) has been used, directly or indirectly, in a manual or
 * automated fashion, to define, examine, create or recreate this structure or any of the data types it
 * depends upon. The reason is obvious, but feel free to ask me if you wonder why.
 */
struct SaveFile {

enum Mount {
    Boat = 0,
    Bird = 4,
    Land = 8,
};

static constexpr byte kNada = -1;

static std::string& sanitize(std::string& name, char rpl = '.');

SaveFile();

void setHeroName(std::string& name);
void setHeroType(Prototype::Type t);
void setHeroLoc(const map::Point& p, unsigned continent = 0); // default/historical:=11,5
void setUIOptions(const UIOptions& opt = UIOptions());
void setLevel(unsigned level); // also initializes timers
void setMission(const Mission& mission, mod::Leftovers& lovers);

// service methods called by setMission
void setMap(unsigned idx, const map::Continent& cont);
void initWanted();
void setGoldenKey(const map::GoldenKey::Burial spot);

char name[kNameSize + 1];
byte type; // A-C, starting from 0; mission option
byte rank; // 0-3, starting from 0
byte intuition; // depends on type
byte education; // depends on type
bool caught[kEnemies];
bool found[kTrophies];
bool avail[kContinents];  // 1 0 0 0
bool visib[kContinents];
byte known[kTechnologies];
bool smart;     // depends on type: type == 2
bool storm;
byte wanted; // = 0xff
char units[kArmySlots]; // = depends on type
byte delay; // interface option = 4
byte level; // mission option
bool sound; // = true
bool blimp; // = true
bool livid; // = true
bool sized; // = true
byte myPoW; // begins with 0
SavedLoc pos;  // starting position
SavedLoc last; // ditto
SavedLoc boat; // = 0
byte boatPofW; // = 0xff initially
byte mount; // = 8/land {0/water, 4/air}
byte cgapal; // =0, irrelevant
char techno[kAlphabet]; // randomized
byte offers[kWantedSlots]; // fill 0-4
byte last_offer; // = 4
byte best_offer; // = 5
byte steps_day; // = 40 (to end, initial)
byte days_week; // = 5  (to end, initial)
char lords[kAlphabet]; // randomize: 127 or enemy number
byte cgate[kAlphabet]; // = 0
byte tgate[kAlphabet]; // = 0
byte key_cont; // XOR
SavedLoc keyl; // XOR
byte visit[kPackedMaps]; // = 0, but prefill w/initial screen: 0x7c 5 times since 0xe4 every 0x8

char gunits[kAlphabet][kArmySlots]; // = depends on enemy placement + randomized for squatters
SavedLoc addme[kContinents][kVolunteers]; // that's all we know about "addmes"
SavedLoc newmaps[kContinents-1]; // no next map at the last continent, obviously
SavedLoc allmaps[kContinents];
SavedLoc tunnels[kContinents][2];
SavedLoc tribes[kContinents][map::kTribes];
SavedLoc idiots[kContinents][map::kIdiots];
char iunits[kContinents][map::kIdiots][kIdiotArmy];
byte itroops[kContinents][map::kIdiots][kIdiotArmy]; // byte!!! not uint16_t!!! (rogue ghosts are trimmed to 8 bits)
char trunits[kContinents][map::kTribes];
byte trtroop[kContinents][map::kTribes];

byte key_ciph;
uint16_t base_command; // depends on mission
uint16_t curr_command; // = ^^^
uint16_t salary; // depends on mission
uint16_t casualties; // = 0

uint16_t troops[kArmySlots];  // = depends on type
uint16_t guards[kAlphabet][kArmySlots]; // = depends on enemy placement + randomized for squatters

uint16_t stasis;
uint16_t d_left; // from time
uint16_t score; // = 0
byte reserved[2]; // = 0
uint32_t money; // == from type
byte maps[kContinents][map::kMapDim][map::kMapDim];

void dump(IO& out, const Aspects& oo = Aspects()) const;

};
#pragma pack()

constexpr unsigned kVisOffset = offsetof(SaveFile, visit);
static_assert(0x0ca == kVisOffset, "Header data must fit in 0xca (202) bytes");

constexpr unsigned kGunOffset = offsetof(SaveFile, gunits);
static_assert(0x8ca == kGunOffset, "Visits data must fit in 0x8ca (2250) bytes");

constexpr unsigned kMapOffset = offsetof(SaveFile, maps);
static_assert(0xfc5 == kMapOffset, "Non-map data must fit in 0xfc5 (4037) bytes");

constexpr unsigned kSavedSize = sizeof(SaveFile); // IDE hover, no other reason to extract
static_assert(0x4fc5 == kSavedSize, "The packed structure must produce a valid DAT file.");

IO& operator<<(IO& out, const SaveFile& sf);

void DumpStats(IO& out);

} // namespace dat

#endif
