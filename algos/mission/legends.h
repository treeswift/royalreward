#ifndef _ALGOS_MISSION_LEGENDS_H_
#define _ALGOS_MISSION_LEGENDS_H_

/**
 * Lore of the land. Provides port and fortress names for correspondence.
 * While we tentatively accept generic terms denoting broadly used board
 * game concepts, all proper names (of places, of characters and of techs)
 * need to be invented anew. See also unit nomenclature in `military.cpp`
 * and ranks in `mission.cpp`.
 * 
 * Since nobody holds rights to the Roman alphabet, or its use in computer
 * interfaces, the letters A-Z continue to be used the way they've been.
 */

namespace loc {

const char* ContName(unsigned index);
const char* PortName(unsigned index);
const char* FortName(unsigned index);
const char* TechName(unsigned index);
const char* TechName(unsigned index, bool combat);
const char* LordName(char code);

} // namespace loc

#endif
