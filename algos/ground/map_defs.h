#ifndef _ALGOS_GROUND_MAP_DEFS_H_
#define _ALGOS_GROUND_MAP_DEFS_H_

#include "precise.h"

namespace map{

constexpr int kMapDim = 64;
constexpr int kMapMax = kMapDim - 1;
constexpr int kMapMem = kMapDim * kMapDim;

constexpr int kMapExt = kMapDim + 1;
constexpr int kExtSqr = kMapExt * kMapExt;

constexpr int kMargin = 1u;
constexpr int kShoalz = 3u;
constexpr int kMEdgez = kMapDim - kShoalz;
constexpr char kColors = 17; // array size

constexpr char kMidSea = 0;
constexpr char kOnLand = 1;
constexpr char kMature = -1; // disable

constexpr int kTribes = 11;
constexpr int kAddMes = 5;   // fixed
constexpr int kIdiots = 35; // fixed

// fixed until here

constexpr char cSpace = '\0';
constexpr char cMagma = '\1';

constexpr char cWater = ' ';
constexpr char cWoods = '@';
constexpr char cRocks = 'M';
constexpr char cSands = '.';
constexpr char cPlain = ':';

constexpr char cCGate = 'F';
constexpr char cCRear = '0';
constexpr char cCMaxF = '9';
constexpr char cCCWLB = '[';
constexpr char cCCWLT = '{';
constexpr char cCCWRB = ']';
constexpr char cCCWRT = '}';

constexpr char cGift1 = '(';
constexpr char cGift2 = ')';

constexpr char cEntry = '/';
constexpr char cHaven = 't';
constexpr char cMetro = 'o';
constexpr char cLabel = '!';
constexpr char cPaper = 'n';
constexpr char cGlass = 'm';
constexpr char cTribe = '&';
constexpr char cAddMe = '+';
constexpr char cChest = '$';
constexpr char cEnemy = '*';

constexpr char cRafts = '#';

constexpr char cPrize = 'v'; // debugging display only

} // namespace map

#endif
