#ifndef _ALGOS_GROUND_MAP_DEFS_H_
#define _ALGOS_GROUND_MAP_DEFS_H_

#include "precise.h"

namespace map{

constexpr unsigned kMapDim = 64u;
constexpr unsigned kMapMax = kMapDim - 1u;
constexpr unsigned kMapMem = kMapDim * kMapDim;

constexpr unsigned kMapExt = kMapDim + 1u;
constexpr unsigned kExtSqr = kMapExt * kMapExt;

constexpr unsigned kMargin = 1u;
constexpr unsigned kShoalz = 3u;
constexpr unsigned kMEdgez = kMapDim - kShoalz;
constexpr unsigned kMinBox = 2u;
constexpr unsigned kFeaAmp = 16u;
constexpr char kColors = 15;
constexpr char kMaxCol = 7; // 1 for Pangaia

constexpr unsigned kNLines = 32u;
constexpr unsigned kDoAcid = 1u;
constexpr unsigned kDoRain = 0u; // >>1 for Polynesia
constexpr unsigned kNLakes = 2u; // 0 for Desertia
constexpr unsigned kEchoes = 5u;

constexpr unsigned kLabels = 11u;
constexpr unsigned kTribes = 11u;
constexpr unsigned kChests = 70u;
constexpr unsigned kAddMes = 5;
constexpr unsigned kChases = 35u;

// decay polynomial
constexpr Real kDecay = 0.0f; // produces archipelagoes
constexpr Real kDPow1 = 0.2f; // produces regular continents
constexpr Real kDPow2 = 0.0f; // produces Laurasia/Gondwana

constexpr Real kWinner = 1.5f;  // reciprocal of threshold
constexpr Real kRugged = 0.f;   // torn edges; rarely needs adjustment
constexpr Real kRoughn = 1e-5f; // log10(rcp(roughness)) is how far from the edges the tear ends
constexpr Real kSmooth = 0.7f;

constexpr unsigned kSeed = 2u;

constexpr char cMagma = '\1';

constexpr char cWater = ' ';
constexpr char cWoods = '@';
constexpr char cRocks = 'M';
constexpr char cSands = '.';
constexpr char cPlain = ':';

constexpr char cCGate = 'F';
constexpr char cCRear = '1';
constexpr char cCCWLB = '[';
constexpr char cCCWLT = '{';
constexpr char cCCWRB = ']';
constexpr char cCCWRT = '}';

constexpr char cGift1 = '(';
constexpr char cGift2 = ')';

constexpr char cEntry = '/';
constexpr char cTower = 't';
constexpr char cMetro = 'o';
constexpr char cLabel = '!';
constexpr char cPaper = 'n';
constexpr char cGlass = 'm';
constexpr char cTribe = '&';
constexpr char cAddMe = '+';
constexpr char cChest = '$';
constexpr char cEnemy = '*';

} // namespace map

#endif
