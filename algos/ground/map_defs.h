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

constexpr int kTribes = 11; // FIXME not really fixed, 11 is only a hard cap. Adjust in Legends!
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

struct Geology {

int kMinBox = 2u;
int kFeaAmp = 16u;
char kMaxCol = 7; // 1 for Pangaia
// TODO: static_assert(kMaxCol+2 < kColors, "tectonic array"); // make a runtime check

char kIsland = kMaxCol + 1; // TODO change to allocCol() call
char kHermit = kIsland + 1; // TODO change to allocCol() call
char kGround = kMidSea; // kIsland;
char kWizard = kIsland;

unsigned kNLines = 32u;
unsigned kDoAcid = 1u;
unsigned kDoRain = 0u; // >>1 for Polynesia
unsigned kNLakes = 2u; // 0 for Desertia
unsigned kEchoes = 5u;

// decay polynomial
Real kDecay = 0.0f; // produces archipelagoes
Real kDPow1 = 0.1f; // produces regular continents
Real kDPow2 = 0.0f; // produces Laurasia/Gondwana

Real kWinner = 1.5f;  // reciprocal of threshold
Real kRugged = 0.f;   // torn edges; rarely needs adjustment
Real kRoughn = 1e-5f; // log10(rcp(roughness)) is how far from the edges the tear ends
Real kSmooth = 0.7f;

unsigned kCastles = 9u; // flexible, but for now can't exceed cCMaxF-cCRear (TODO validate)
unsigned kLabels = 22u; // resource-determined
unsigned kChests = 70u; // flexible, but default=good

// realy-really very last touches
bool kAridize = false;
char kSuomize = false; // other valid options are cWoods and cRocks
    
};

} // namespace map

#endif
