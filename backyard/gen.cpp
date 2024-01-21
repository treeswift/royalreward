#include "geometry.h"
#include "geography.h"
#include "goldenkey.h"

#include <cstdio>

using namespace map;

constexpr int kSeed = 1u;

int main(int argc, char** argv) {
    using namespace map;
    (void) argc;
    (void) argv;

    rnd::seed(kSeed);
    Continent cont;
    cont.generate();

    GoldenKey gk;
    gk.consider(cont);
    map::Point k = gk.select().p;

    // replace the following with *stdout << gk
    char c = cPrize;
    char& pl = at(cont.map, k);
    std::swap(c, pl);
    // wrap the following in *stdout << continent
    // cont.maskCities(false);
    *stdout << cont.map;
    // cont.maskCities();
    std::swap(c, pl); // ... gk

    // TODO add map tuning dump

    return 0;
}
