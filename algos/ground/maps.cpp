#include "maps.h"

#include <vector>

namespace map {

static std::vector<Shift> side_neighbor = {
    {0,1}, {1,0}, {0, -1}, {-1, 0}
};
static std::vector<Shift> vert_neighbor = {
    {0,1}, {1,0}, {0, -1}, {-1, 0},
    {1,1}, {1,-1}, {-1,1}, {-1,-1},
};

void paint(unsigned x, unsigned y, bool_xy test, with_xy flip, const std::vector<Shift>& nb) {
    if(test(x, y)) {
        flip(x, y);
        for(const Shift& d : nb) {
            paint(x + d.dx, y + d.dy, test, flip, nb);
        }
    }
}

with_xy paint4(bool_xy test, with_xy flip) {
    return [=]WITH_XY {
        paint(x, y, test, flip, side_neighbor);
    };
}

with_xy paint8(bool_xy test, with_xy flip) {
    return [=]WITH_XY {
        paint(x, y, test, flip, vert_neighbor);
    };
}

} // namespace map

