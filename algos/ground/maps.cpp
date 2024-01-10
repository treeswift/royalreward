#include "maps.h"

#include <vector>

struct Dir { int dx, dy; };
std::vector<Dir> side_neighbor = {
    {0,1}, {1,0}, {0, -1}, {-1, 0}
};
std::vector<Dir> vert_neighbor = {
    {0,1}, {1,0}, {0, -1}, {-1, 0},
    {1,1}, {1,-1}, {-1,1}, {-1,-1},
};

namespace map {

void for_rect(unsigned x0, unsigned y0, unsigned xm, unsigned ym, with_xy op) {
    for(unsigned y = y0; y < ym; ++y) {
        for(unsigned x = x0; x < xm; ++x) {
            op(x, y);
        }
    }
}

void paint(unsigned x, unsigned y, bool_xy test, with_xy flip, const std::vector<Dir>& nb) {
    if(test(x, y)) {
        flip(x, y);
        for(const Dir& d : nb) {
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

