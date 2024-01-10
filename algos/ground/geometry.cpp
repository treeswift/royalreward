#include "geometry.h"

namespace map {

Shift& Shift::operator+=(const Shift& delta) {
    dx += delta.dx;
    dy += delta.dy;
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

Point& Point::operator+=(const Shift& shift) {
    x += shift.dx;
    y += shift.dy;
    return *this;
}

Point operator+(const Point& base, const Shift& shift) {
    Point copy = base;
    return copy += shift;
}

Block block(const Point& base, const Shift& size) {
    return {base, base + size};
}

Block square(const Point& base, unsigned side) {
    return {base, base + Shift::diag(side)};
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
