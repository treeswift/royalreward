#include "aynrand.h"

namespace rnd {

constexpr bool kTrivial = false;

std::ranlux24_base engine;
std::uniform_real_distribution<> distro{0., 1.};

void seed(unsigned seed) {
    std::srand(seed);
    engine.seed(seed);
}

float zto1() {
    return distro(engine);
}

unsigned upto(unsigned upper) {
    return kTrivial ? std::rand() % upper : std::uniform_int_distribution<>(0, upper - 1u)(engine);
}

}