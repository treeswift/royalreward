#ifndef _ALGOS_GROUND_MAPS_H_
#define _ALGOS_GROUND_MAPS_H_

#include "map_defs.h"

#include <functional>
#include <vector>

namespace map{

template<typename T> using Map = T[kMapExt][kMapExt];

using ChrMap = Map<char>;
using EleMap = Map<Real>;

template<typename T>
struct MapHolder {
    MapHolder(const T& def) {
        vec.resize(kExtSqr, def);
    }
    MapHolder(const MapHolder<T>&) = default;
    MapHolder& operator= (const MapHolder<T>&) = default;
    const Map<T>& map() const {
        return *reinterpret_cast<const Map<T>*>(vec.data());
    }    
    Map<T>& map() {
        return *reinterpret_cast<Map<T>*>(vec.data());
    }
private:
    std::vector<T> vec;
};

inline bool isinner(unsigned x, unsigned y) {
    return x >= kMargin && y >= kMargin && x < kMapMax && y < kMapMax;
}

using with_xy = std::function<void(unsigned, unsigned)>;
using bool_xy = std::function<bool(unsigned, unsigned)>;
#define WITH_XY (unsigned x, unsigned y)

void for_rect(unsigned x0, unsigned y0, unsigned xm, unsigned ym, with_xy op);

with_xy paint4(bool_xy test, with_xy flip);
with_xy paint8(bool_xy test, with_xy flip);

} // namespace map

#endif
