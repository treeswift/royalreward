#ifndef _ALGOS_GROUND_MAP_DEFS_H_
#define _ALGOS_GROUND_MAP_DEFS_H_

using Real = float; // double produces no real improvement atm

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
constexpr char kMaxCol = 7;

constexpr unsigned kNLines = 32u;
constexpr unsigned kDoAcid = 0u;  // TODO softcode, test <3 on cont#3
constexpr unsigned kDoRain = 0u;  // TODO softcode, test <1 on cont#3
constexpr unsigned kNLakes = 64u; // MOREINFO correlate with castles?

// decay polynomial
constexpr Real kDecay = 0.0f; // produces archipelagoes
constexpr Real kDPow1 = 0.0f; // produces regular continents
constexpr Real kDPow2 = 0.1f; // produces Laurasia/Gondwana

constexpr Real kWinner = 1.5f;  // reciprocal of threshold
constexpr Real kRugged = 0.f;   // TODO hardcode to 0 when stabilized
constexpr Real kRoughn = 1e-5f; // log10(rcp(roughness)) is how far from the edges the tear ends
constexpr Real kSmooth = 0.7f;

constexpr unsigned kSeed = 2u; // TODO use libc-independent rand()

constexpr char cMagma = '\1';

constexpr char cWater = ' ';
constexpr char cWoods = '@';
constexpr char cRocks = '^';
constexpr char cSands = '.';
constexpr char cPlain = ':';

constexpr char cCGate = 'F';
constexpr char cCRear = '1';
constexpr char cCCWLT = '[';
constexpr char cCCWLB = '{';
constexpr char cCCWRT = ']';
constexpr char cCCWRB = '}';

constexpr char cEntry = '/';


} // namespace map

#endif
