#ifndef _ALGOS_GROUND_MAP_DEFS_H_
#define _ALGOS_GROUND_MAP_DEFS_H_

#include "precise.h"

namespace map{

constexpr int kMapDim = 64u;
constexpr int kMapMax = kMapDim - 1u;
constexpr int kMapMem = kMapDim * kMapDim;

constexpr int kMapExt = kMapDim + 1u;
constexpr int kExtSqr = kMapExt * kMapExt;

constexpr int kMargin = 1u;
constexpr int kShoalz = 3u;
constexpr int kMEdgez = kMapDim - kShoalz;
constexpr int kMinBox = 2u;
constexpr int kFeaAmp = 16u;
constexpr char kColors = 15;
constexpr char kMaxCol = 7; // 1 for Pangaia
static_assert(kMaxCol+1 < kColors, "tectonic array");

constexpr char kMidSea = 0;
constexpr char kOnLand = 1;
constexpr char kIsland = kMaxCol + 1;
constexpr char kMature = '\255'; // disable
constexpr char kGround = kMidSea; // kIsland;
constexpr bool kWizard = true;

constexpr int kNLines = 32u;
constexpr int kDoAcid = 1u;
constexpr int kDoRain = 0u; // >>1 for Polynesia
constexpr int kNLakes = 2u; // 0 for Desertia
constexpr int kEchoes = 5u;

constexpr int kCastles = 9u;
constexpr int kLabels = 11u;
constexpr int kTribes = 11u;
constexpr int kChests = 70u;
constexpr int kAddMes = 5;
constexpr int kIdiots = 35u;

// decay polynomial
constexpr Real kDecay = 0.0f; // produces archipelagoes
constexpr Real kDPow1 = 0.1f; // produces regular continents
constexpr Real kDPow2 = 0.0f; // produces Laurasia/Gondwana

constexpr Real kWinner = 1.5f;  // reciprocal of threshold
constexpr Real kRugged = 0.f;   // torn edges; rarely needs adjustment
constexpr Real kRoughn = 1e-5f; // log10(rcp(roughness)) is how far from the edges the tear ends
constexpr Real kSmooth = 0.7f;

constexpr int kSeed = 1u;

constexpr char cMagma = '\1';

constexpr char cWater = ' ';
constexpr char cWoods = '@';
constexpr char cRocks = 'M';
constexpr char cSands = '.';
constexpr char cPlain = ':';

constexpr char cCGate = 'F';
constexpr char cCRear = '0';
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

constexpr char cShaft = '#';

constexpr char cPrize = 'v'; // debugging display only

} // namespace map

#endif
