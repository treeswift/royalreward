#ifndef _ALGOS_GROUND_CHOICES_H_
#define _ALGOS_GROUND_CHOICES_H_

#include "precise.h"
#include <functional>

// sorting, ranking and weighted selection

namespace rnk {

using Rate = std::function<Real(unsigned)>;

struct Pick
{
    unsigned sel;
    Real weight;
};

Pick pickWeighed(unsigned wcount, Rate weigh, Real favor_last = 1.f);

}

#endif