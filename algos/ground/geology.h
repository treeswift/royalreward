#ifndef _ALGOS_GROUND_GEOLOGY_H_
#define _ALGOS_GROUND_GEOLOGY_H_

// this structure is the made-mutable part of...
#include "map_defs.h"

namespace map {

struct Geology {

int kMinBox = 2;
int kFeaAmp = 16;
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

unsigned kCastles = cCMaxF-cCRear; // default = max
unsigned kRoamers = kIdiots; // flexible, ad hoc
unsigned kDwells = kTribes;
unsigned kLabels = 22u; // resource-determined
unsigned kChests = 70u; // flexible, but default=good

// realy-really very last touches
bool kAridize = false;
char kSuomize = false; // other valid options are cWoods and cRocks
bool kSandboat = false; // "true" allows desert to border the ocean

// constructors and mutators
Geology(const Geology&) = default;
Geology& operator=(const Geology&) = default;

Geology(int elements = 7);
void setComposition(int elements);

};

} // namespace map 

#endif
