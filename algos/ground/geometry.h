#ifndef _ALGOS_GROUND_GEOMETRY_H_
#define _ALGOS_GROUND_GEOMETRY_H_

#include <functional>

namespace map {

struct Shift {
    int dx, dy;

    Shift& operator+=(const Shift& delta);
    Shift& operator*=(int factor);
    Shift operator-() const;

    static Shift hori(int side) { return {side, 0}; }
    static Shift vert(int side) { return {0, side}; }
    static Shift diag(int side) { return {side, side}; }
};

Shift operator*(const Shift& base, int factor);
Shift operator+(const Shift& left, const Shift& right);

struct Point {
    unsigned x, y;
    
    Point& operator+=(const Shift& shift);
};

Point operator+(const Point& base, const Shift& shift);

struct Block {
    Point base, upto;

    // TODO iteration
};

Block block(const Point& base, const Shift& size);
Block square(const Point& base, unsigned side);
Block bound(unsigned base, unsigned upto);

using with_xy = std::function<void(unsigned, unsigned)>;
using bool_xy = std::function<bool(unsigned, unsigned)>;
#define WITH_XY (unsigned x, unsigned y)

void for_rect(unsigned x0, unsigned y0, unsigned xm, unsigned ym, with_xy op);

} // namespace map

#endif
