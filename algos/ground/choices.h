#ifndef _ALGOS_GROUND_CHOICES_H_
#define _ALGOS_GROUND_CHOICES_H_

#include "precise.h"
#include "geometry.h"
#include <functional>
#include <map>

// sorting, ranking and weighted selection

namespace rnk {

using Rate = std::function<Real(unsigned)>;

struct Pick
{
    unsigned sel;
    Real weight;
};

Pick pickWeighed(unsigned wcount, Rate weigh, Real favor_last = 1.f);

template<typename T>
using Ranked = std::multimap<Real, T>;

using RankFN = std::function<Real WITH_XY>;
using SweetP = rnk::Ranked<map::Point>;

SweetP sweetSpots(const map::Block& block, RankFN rating);

using PrediP = std::function<bool(const map::Point&)>;
using PlaceP = std::function<char(char, const map::Point& p)>;

}

#endif
