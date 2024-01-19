#include "choices.h"

#include "aynrand.h"
#include <vector>

namespace rnk
{

Pick pickWeighed(unsigned wcount, Rate weigh, Real favor_last) {
    std::vector<Real> weights;
    Real total_weight = 0.f;
    for(unsigned j = 0; j < wcount; ++j) {
        Real weight = weigh(j);
        weights.push_back(weight);
        total_weight += weight;
    }
    Real cast = rnd::zto1() * total_weight * favor_last;
    unsigned sel = wcount - 1;
    for(unsigned j = 0; j < wcount; ++j) {
        if((cast -= weights[j]) <= 0.f) {
            sel = j;
            break;
        }
    }
    return {sel, weights[sel]};
}

SweetP sweetSpots(const map::Block& block, RankFN rating) {
    Ranked<map::Point> sweetspots;
    block.visit([&]WITH_XY {
        Real sugar = rating(x, y);
        if(sugar > 0.f) {
            sweetspots.insert({sugar, map::Point{x, y}});
        }
    });
    return sweetspots;
}

}