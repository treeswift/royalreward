#ifndef _ALGOS_GROUND_AYNRAND_H_
#define _ALGOS_GROUND_AYNRAND_H_

#include "precise.h"

#include <memory>

namespace rnd {

unsigned hwrandom(); // use hardware random device to generate a single value

class Rand;

struct Ayn {
    // shallow copies
    Ayn(const Ayn& dup) = default;
    Ayn& operator=(const Ayn& dup) = default;

    Ayn(unsigned seed = hwrandom());
    unsigned seed() const;
    void seed(unsigned seed);

    int upto(int upper);
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

private:
    std::shared_ptr<Rand> rand;
};

Ayn& ein(); // eingleton

inline void seed(unsigned seed) { ein().seed(seed); }

inline int upto(int upper) { return ein().upto(upper); }

inline int inrg(int lower, int upper) { return ein().inrg(lower, upper); }

inline Real zto1() { return ein().zto1(); }

template<typename C>
void shuffle(C& c, unsigned preserve = 0u) {
    return ein().shuffle<C>(c, preserve);
}

} // namespace rnd

#endif
