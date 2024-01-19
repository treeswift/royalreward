#include "geometry.h"
#include "aynrand.h"

#include <cmath>

namespace map {

int Shift::d2() const { return dx*dx + dy*dy; }

Real Shift::d() const { return std::sqrt((Real) d2()); }

Shift& Shift::operator+=(const Shift& delta) {
    dx += delta.dx;
    dy += delta.dy;
    return *this;
}

Shift& Shift::operator-=(const Shift& subtr) {
    dx -= subtr.dx;
    dy -= subtr.dy;
    return *this;
}

Shift& Shift::operator*=(int factor) {
    dx *= factor;
    dy *= factor;
    return *this;
}

Shift Shift::operator-() const {
    return *this * -1;
}

Shift operator*(const Shift& base, int factor) {
    Shift copy = base;
    return copy *= factor;
}

Shift operator+(const Shift& left, const Shift& right) {
    Shift copy = left;
    return copy += right;
}

Shift operator-(const Shift& left, const Shift& right) {
    Shift copy = left;
    return copy -= right;
}

Shift Shift::right() const {
    return {-dy, dx};
}

Shift Shift::left() const {
    return {dy, -dx};
}

Shift Shift::rand() const {
    return {rnd::upto(dx), rnd::upto(dy)};
}

Shift hori(int side) { return {side, 0}; }
Shift vert(int side) { return {0, side}; }
Shift diag(int side) { return {side, side}; }

Point& Point::operator+=(const Shift& shift) {
    x += shift.dx;
    y += shift.dy;
    return *this;
}

Point& Point::operator-=(const Shift& shift) {
    x -= shift.dx;
    y -= shift.dy;
    return *this;
}

Point operator+(const Point& base, const Shift& shift) {
    Point copy = base;
    return copy += shift;
}

Point operator-(const Point& base, const Shift& shift) {
    Point copy = base;
    return copy -= shift;
}

Shift Point::operator-(const Point& base) const {
    return {x - base.x, y - base.y};
}

Point Point::blend(const Point& other, Real f) const {
    return *this + (other - *this) * f;
}

bool Point::operator==(const Point& other) const {
    return x == other.x && y == other.y;
}

Point corner(int far) { return {far, far}; }

Block block(const Point& base, const Shift& size) {
    return {base, base + size};
}

Block square(const Point& base, int side) {
    return {base, base + diag(side)};
}

Block bound(int base, int upto) {
    return {{base, base}, {upto, upto}};
}

Block& Block::operator&=(const Block& other) {
    base.x = std::max(base.x, other.base.x);
    base.y = std::max(base.y, other.base.y);
    upto.x = std::min(upto.x, other.upto.x);
    upto.y = std::min(upto.y, other.upto.y);
    return *this;
}

Block operator&(const Block& base, const Block& other) {
    Block copy = base; return copy &= other;
}

void for_rect(int x0, int y0, int xm, int ym, with_xy op) {
    for(int y = y0; y < ym; ++y) {
        for(int x = x0; x < xm; ++x) {
            op(x, y);
        }
    }
}

Block nearby(const Point& p) {
    return square(p, 1).inset(-1);
}

} // namespace map
