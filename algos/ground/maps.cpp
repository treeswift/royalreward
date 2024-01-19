#include "maps.h"

#include <vector>
#include <string>

namespace map {

static std::vector<Shift> side_neighbor = {
    {0,1}, {1,0}, {0, -1}, {-1, 0}
};
static std::vector<Shift> vert_neighbor = {
    {0,1}, {1,0}, {0, -1}, {-1, 0},
    {1,1}, {1,-1}, {-1,1}, {-1,-1},
};

void paint(int x, int y, bool_xy test, with_xy flip, const std::vector<Shift>& nb) {
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

Block screen(const Point& p) {
    return square(p, 1).inset(-2);
}

std::ostream& operator<<(std::ostream& out, const ChrMap& map) {
    for(int y = kMapDim; y; ) {
        --y;
        std::string line;
        for(int x = 0; x < kMapDim; ++x) {
            char c = map[y][x];
            out << c << c;
        }
        out << std::endl;
    }
    return out;
}

IO& operator<<(IO& out, const ChrMap& map) {
    for(int y = kMapDim; y; ) {
        --y;
        std::string line;
        for(int x = 0; x < kMapDim; ++x) {
            line.push_back(map[y][x]);
            line.push_back(map[y][x]);
        }
        fprintf(&out, "%s\n", line.c_str());
    }
    return out;
}

} // namespace map

