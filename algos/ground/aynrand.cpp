#include "aynrand.h"

#include "mumbling.h"

#include <cstdlib> // legacy std::rand
#include <random>  // advanced engines

namespace rnd {

constexpr bool kTrivial = false;

std::ranlux24_base engine;
std::uniform_real_distribution<> distro{0., 1.};

std::random_device fresh;

unsigned hwrandom() {
    return std::uniform_int_distribution<>(0, UINT32_MAX)(fresh);
}

void seed(unsigned seed) {
    std::srand(seed);
    engine.seed(seed);
}

Real zto1() {
    return distro(engine);
}

unsigned upto(int upper) {
    if(upper <= 0) {
        mum::bummer<std::underflow_error>("rnd::upto(%d)", upper);
    }
    return kTrivial ? std::rand() % upper : std::uniform_int_distribution<>(0, upper - 1u)(engine);
}

}