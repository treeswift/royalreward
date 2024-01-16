#ifndef _ALGOS_GROUND_GEOMETRY_H_
#define _ALGOS_GROUND_GEOMETRY_H_

#include "precise.h"
#include "aynrand.h"

#include <functional>

namespace map {

struct Shift {
    int dx, dy;

    int d2() const;
    Real d() const;

    Shift& operator+=(const Shift& delta);
    Shift& operator*=(int factor);
    Shift& operator-=(const Shift& subtr);
    Shift operator-() const;
    Shift right() const;
    Shift left() const;
    Shift rand() const;
};

Shift hori(int side);
Shift vert(int side);
Shift diag(int side);

Shift operator*(const Shift& base, int factor);
Shift operator+(const Shift& left, const Shift& right);
Shift operator-(const Shift& left, const Shift& right);

struct Point {
    int x, y;
    
    Point& operator+=(const Shift& shift);
    Point& operator-=(const Shift& shift);

    Shift operator-(const Point& base) const;
    Point blend(const Point& other, Real f) const;

    bool operator==(const Point& other) const;
};

Point operator+(const Point& base, const Shift& shift);
Point operator-(const Point& base, const Shift& shift);

Point corner(int far);

using with_xy = std::function<void(int, int)>;
using bool_xy = std::function<bool(int, int)>;
#define WITH_XY (int x, int y)

void for_rect(int x0, int y0, int xm, int ym, with_xy op);

struct Block {
    Point base, upto;

    Shift size() const { return upto - base; }

    Point rand() const {
        return base + size().rand();
    }

    bool covers(const Point& p) const {
        return p.x >= base.x && p.y >= base.y && p.x < upto.x && p.y < upto.y;
    }

    void visit(with_xy op) {
        for_rect(base.x, base.y, upto.x, upto.y, op);
    }

    Block inset(int inset) const {
        return {base + diag(inset), upto - diag(inset)};
    }

    Block& operator&=(const Block& other);
};

Block block(const Point& base, const Shift& size);
Block square(const Point& base, int side=1u);
Block bound(int base, int upto);
Block operator&(const Block& base, const Block& other);

} // namespace map

#endif
