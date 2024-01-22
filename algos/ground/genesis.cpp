#include "geography.h"

#include <cstdio>
#include <iostream>
#include <random>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <functional>

#include "aynrand.h"
#include "choices.h"
#include "maps.h"

namespace map {

/**
 * Generate a map.
 */

void Continent::generate() {
    formLand();
    segregate();
    makeLakes();
    castleize();
    paveRoads();
    markHome();
    polish();
    petrify(); // petrification is conservative/transactional => works on a polished surface
    segment(); // second polish included inside
    //
    markGates();
    specials();
    citymize();
    cconnect();
}

} // namespace map
