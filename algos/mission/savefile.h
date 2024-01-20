#ifndef _ALGOS_MISSION_SAVEFILE_H_
#define _ALGOS_MISSION_SAVEFILE_H_

#include "map_defs.h"
#include "dat_defs.h"
#include "maps.h"
#include "mission.h"
#include "geography.h"
#include "goldenkey.h"

#include <cstdint>

namespace dat {

/**
 * The binary layout of the following data structure is based on a verbal description from ModdingWiki:
 *  https://moddingwiki.shikadi.net/wiki/King%27s_Bounty_Saved_game_Format
 *  https://moddingwiki.shikadi.net/wiki/King%27s_Bounty_Map_Format
 *
 * No copyrighted code (from OpenKB or otherwise) has been used, directly or indirectly, in a manual or
 * automated fashion, to define, examine, create or recreate this structure or any of the data types it
 * depends upon. The reason is obvious, but feel free to ask me if you wonder why.
 */

constexpr unsigned kArmySlots = 5;
constexpr unsigned kIdiotArmy = 3;
constexpr unsigned kWantedSlots = 5;
constexpr unsigned kPackedMaps = (map::kMapMem * kPartsOfWorld) >> 3;
constexpr unsigned kkVolunteers = map::kAddMes;

struct UIOptions {}; // move to UI

#pragma pack(1)
struct SavedLoc {
    char x, y;

    SavedLoc& operator=(const map::Point& p);
};

struct SaveFile {

enum Mount {
    Boat = 0,
    Bird = 4,
    Land = 8,
};

static constexpr char kNada = -1;

SaveFile();

void setHeroName(const std::string& name);
void setHeroType(Prototype::Type t);
void setHeroLoc(const map::Point& p);
void setUIOptions(UIOptions opt = {});
void setLevel(); // also initializes 
void setMap(unsigned idx, const map::Continent& cont);
void setEnemies(); // also initializes offers/rewards
void setGoldenKey(const map::GoldenKey::Burial spot);

char name[11];
char type; // A-C, starting from 0; mission option
char rank; // 0-3, starting from 0
char intuition; // depends on type
char education; // depends on type
bool caught[kEnemies];
bool found[kTrophies];
bool avail[kPartsOfWorld];  // 1 0 0 0
bool visib[kPartsOfWorld];
char known[kTechnologies];
bool smart;     // depends on type: type == 2
bool storm;
char wanted; // = 0xff
char units[kArmySlots]; // = depends on type
char delay; // interface option = 4
char level; // mission option
bool sound; // = true
bool blimp; // = true
bool livid; // = true
bool sized; // = true
char myPoW; // begins with 0
SavedLoc pos;  // starting position
SavedLoc last; // ditto
SavedLoc boat; // = 0
char boatPofW; // = 0xff initially
char mount; // = 8/land {0/water, 4/air}
char cgapal; // =0, irrelevant
char techno[kAlphabet]; // randomized
char offers[kWantedSlots]; // fill 0-4
char last_offer; // = 4
char best_offer; // = 5
char steps_day; // = 40 (to end, initial)
char days_week; // = 5  (to end, initial)
char lords[kAlphabet]; // randomize: 127 or enemy number
char cgate[kAlphabet]; // = 0
char tgate[kAlphabet]; // = 0
char key_part; // XOR
SavedLoc keyl; // XOR
char visit[kPackedMaps]; // = 0, but prefill w/initial screen: 0x7c 5 times since 0xe4 every 0x8

char gunits[kAlphabet][kArmySlots]; // = depends on enemy placement + randomized for squatters
SavedLoc addme[kPartsOfWorld][kkVolunteers]; // that's all we know about "addmes"
SavedLoc newmaps[kPartsOfWorld-1]; // no next map at the last continent, obviously
SavedLoc allmaps[kPartsOfWorld];
SavedLoc tunnels[kPartsOfWorld][2];
SavedLoc tribes[kPartsOfWorld][map::kTribes];
SavedLoc idiots[kPartsOfWorld][map::kIdiots];
char iunits[kPartsOfWorld][map::kIdiots][kIdiotArmy];
char itroops[kPartsOfWorld][map::kIdiots][kIdiotArmy]; // char!!! not uint16_t!!! (are rogue ghosts rounded up? verify!)
char trunits[kPartsOfWorld][map::kTribes];
char trtroop[kPartsOfWorld][map::kTribes];

char key_ciph;
uint16_t base_command; // depends on mission
uint16_t curr_command; // = ^^^
uint16_t salary; // depends on mission
uint16_t casualties; // = 0

uint16_t troops[kArmySlots];  // = depends on type
uint16_t guards[kAlphabet][kArmySlots]; // = depends on enemy placement + randomized for squatters

uint16_t stasis;
uint16_t d_left; // from time
uint16_t score; // = 0
char reserved[2]; // = 0
uint32_t money; // == from mission
char maps[kPartsOfWorld][map::kMapDim][map::kMapDim];

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

using IO = decltype(*stdout);
IO& operator<<(IO& out, const SaveFile& map);

} // namespace dat

#endif
