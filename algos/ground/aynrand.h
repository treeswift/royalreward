#ifndef _ALGOS_GROUND_AYNRAND_H_
#define _ALGOS_GROUND_AYNRAND_H_

#include <cstdlib> // legacy std::rand
#include <random>  // advanced engines

namespace rnd {

inline int upto(int upper) { return std::rand() % upper; }

inline int inrg(int lower, int upper) { return upto(upper - lower) + lower; }

}

#endif
