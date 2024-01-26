#include "aynrand.h"

#include "mumbling.h"

#include <cstdlib> // legacy std::rand
#include <random>  // advanced engines

namespace rnd {

constexpr bool kTrivial = false;

class Rand {
    std::ranlux24_base engine;
    std::uniform_real_distribution<> distro{0., 1.};

public:
    void seed(unsigned seed) {
        std::srand(seed);
        engine.seed(seed);
    }

    int upto(int upper) {
        if(upper <= 0) {
            mum::bummer<std::underflow_error>("rnd::upto(%d)", upper);
        }
        return kTrivial ? std::rand() % upper : std::uniform_int_distribution<>(0, upper - 1u)(engine);
    }

    Real zto1() {
        return distro(engine);
    }
};

std::random_device fresh;

Ayn::Ayn() : rand(std::make_shared<Rand>()) {}

Ayn& Ayn::ein() {
    static Ayn ayn;
    return ayn;
}

unsigned hwrandom() {
    return std::uniform_int_distribution<>(0, UINT32_MAX)(fresh);
}

void Ayn::seed(unsigned seed) {
    rand->seed(seed);
}

Real Ayn::zto1() {
    return rand->zto1();
}

int Ayn::upto(int upper) {
    return rand->upto(upper);
}

}