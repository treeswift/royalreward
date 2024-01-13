#ifndef _ALGOS_GROUND_AYNRAND_H_
#define _ALGOS_GROUND_AYNRAND_H_

#include <cstdlib> // legacy std::rand
#include <random>  // advanced engines

namespace rnd {

void seed(unsigned seed);

unsigned upto(unsigned upper);

float zto1();

inline int inrg(int lower, int upper) { return upto(upper - lower) + lower; }

}

#endif
