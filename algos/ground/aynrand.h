#ifndef _ALGOS_GROUND_AYNRAND_H_
#define _ALGOS_GROUND_AYNRAND_H_

#include "precise.h"

#include <memory>

namespace rnd {

class Rand;

class Ayn {
public:
    static Ayn& ein();

    Ayn();

    void seed(unsigned seed);
    int upto(int upper);
    Real zto1();

    template<typename C>
    void shuffle(C& c, unsigned preserve = 0u) {
        unsigned size = c.size();
        for(unsigned s = 0; s < size; ++s) {
            unsigned i = inrg(preserve, size), j = inrg(preserve, size);
            auto tmp = c[i]; c[i] = c[j]; c[j] = tmp;
        }
    }

    inline int inrg(int lower, int upper) { return upto(upper - lower) + lower; }

private:
    std::shared_ptr<Rand> rand;
};

inline void seed(unsigned seed) { Ayn::ein().seed(seed); }

inline int upto(int upper) { return Ayn::ein().upto(upper); }

inline int inrg(int lower, int upper) { return Ayn::ein().inrg(lower, upper); }

inline Real zto1() { return Ayn::ein().zto1(); }

template<typename C>
void shuffle(C& c, unsigned preserve = 0u) {
    return Ayn::ein().shuffle<C>(c, preserve);
}

unsigned hwrandom(); // use hardware random device to generate a single value

}

#endif
