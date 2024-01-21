#ifndef _ALGOS_MISSION_DAT_DEFS_H_
#define _ALGOS_MISSION_DAT_DEFS_H_

namespace dat {

constexpr unsigned kAlphabet = 26;
constexpr unsigned kContinents = 4;
constexpr unsigned kTechControls = 7;
constexpr unsigned kTechnologies = kTechControls * 2;
constexpr unsigned kEnemies = 0x11;
constexpr unsigned kTrophies = 0x8;

constexpr unsigned kArmySlots = 5;
constexpr unsigned kIdiotArmy = 3;

constexpr unsigned kNameSize = 10;

// ad hoc convention we follow so far
constexpr char kCleansed = -1;
constexpr char kSquatter = 0x7f;

} // namespace dat

#endif
