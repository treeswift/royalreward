#ifndef _ALGOS_GROUND_GEOMETRY_H_
#define _ALGOS_GROUND_GEOMETRY_H_

#include <functional>
#include "aynrand.h"

namespace map {

struct Shift {
    int dx, dy;

    int d2() const;
    float d() const;

    Shift& operator+=(const Shift& delta);
    Shift& operator*=(int factor);
    Shift& operator-=(const Shift& subtr);
    Shift operator-() const;

    Shift rand() const;
};

Shift hori(int side);
Shift vert(int side);
Shift diag(int side);

Shift operator*(const Shift& base, int factor);
Shift operator+(const Shift& left, const Shift& right);
Shift operator-(const Shift& left, const Shift& right);

struct Point {
    unsigned x, y;
    
    Point& operator+=(const Shift& shift);
    Point& operator-=(const Shift& shift);

    Shift operator-(const Point& base) const {
        return {x - base.x, y - base.y};
    }
};

Point operator+(const Point& base, const Shift& shift);
Point operator-(const Point& base, const Shift& shift);

Point corner(unsigned far);

using with_xy = std::function<void(unsigned, unsigned)>;
using bool_xy = std::function<bool(unsigned, unsigned)>;
#define WITH_XY (unsigned x, unsigned y)

void for_rect(unsigned x0, unsigned y0, unsigned xm, unsigned ym, with_xy op);

struct Block {
    Point base, upto;

    Shift size() const { return upto - base; }

    Point rand() const {
        return base + size().rand();
    }

    void visit(with_xy op) {
        for_rect(base.x, base.y, upto.x, upto.y, op);
    }

    Block inset(unsigned inset) const {
        return {base + diag(inset), upto - diag(inset)};
    }
};

Block block(const Point& base, const Shift& size);
Block square(const Point& base, unsigned side);
Block bound(unsigned base, unsigned upto);

} // namespace map

#endif
