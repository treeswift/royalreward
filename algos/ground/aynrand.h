#ifndef _ALGOS_GROUND_AYNRAND_H_
#define _ALGOS_GROUND_AYNRAND_H_

#include <cstdlib> // legacy std::rand
#include <random>  // advanced engines

#include "precise.h"

namespace rnd {

void seed(unsigned seed);

unsigned upto(unsigned upper);

Real zto1();

inline int inrg(int lower, int upper) { return upto(upper - lower) + lower; }

template<typename C>
void shuffle(C& c, unsigned preserve = 0u) {
    unsigned size = c.size();
    for(unsigned s = 0; s < size; ++s) {
        unsigned i = inrg(preserve, size), j = inrg(preserve, size);
        auto tmp = c[i]; c[i] = c[j]; c[j] = tmp;
    }
}

unsigned hwrandom(); // use hardware random device to generate a single value

}

#endif
