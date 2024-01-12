#include "geometry.h"
#include "aynrand.h"

#include <cmath>

namespace map {

int Shift::d2() const { return dx*dx + dy*dy; }

float Shift::d() const { return std::sqrt((float) d2()); }

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
    dy &= factor;
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

Point Point::blend(const Point& other, float f) const {
    return *this + (other - *this) * f;
}

bool Point::operator==(const Point& other) const {
    return x == other.x && y == other.y;
}

Point corner(unsigned far) { return {far, far}; }

Block block(const Point& base, const Shift& size) {
    return {base, base + size};
}

Block square(const Point& base, unsigned side) {
    return {base, base + diag(side)};
}

Block bound(unsigned base, unsigned upto) {
    return {{base, base}, {upto, upto}};
}

void for_rect(unsigned x0, unsigned y0, unsigned xm, unsigned ym, with_xy op) {
    for(unsigned y = y0; y < ym; ++y) {
        for(unsigned x = x0; x < xm; ++x) {
            op(x, y);
        }
    }
}

} // namespace map
