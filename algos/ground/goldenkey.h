#ifndef _ALGOS_GROUND_GOLDENKEY_H_
#define _ALGOS_GROUND_GOLDENKEY_H_

#include "aynrand.h"
#include "geometry.h"
#include "geography.h"
#include "maps.h"

#include <map>
#include <vector>

namespace map {

struct GoldenKey {
    struct Burial { Point p; unsigned cindex = 0; unsigned ucount = 0; bool unique = true; };
    Block shelf;

    // service methods
    static char convert(char c);
    static std::string look(const ChrMap& map, const Point& p);

    // population
    void consider(const Continent& cont);

    // selection
    Burial select(rnd::Ayn& rnd = rnd::ein()) const;

    const Burial& selected() const {
        return spot;
    }

    unsigned stats_locs = 0;
    unsigned stats_puts = 0;
    unsigned stats_rejc = 0;

private:
    mutable Burial spot;
    std::vector<const ChrMap*> maps;
    std::map<std::string, Burial> papermaps;
};

} // namespace map

#endif