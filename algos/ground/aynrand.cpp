#include "aynrand.h"

namespace rnd {

constexpr bool kTrivial = false;

std::mt19937 engine;
std::uniform_real_distribution<> distro{0., 1.};

void seed(unsigned seed) {
    std::srand(seed);
    engine.seed(seed);
}

unsigned upto(unsigned upper) {
    return kTrivial ? std::rand() % upper : std::uniform_int_distribution<>(0, upper)(engine);
}

}