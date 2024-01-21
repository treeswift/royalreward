#ifndef _ALGOS_GROUND_GEOGRAPHY_H_
#define _ALGOS_GROUND_GEOGRAPHY_H_

#include "geometry.h"
#include "choices.h"
#include "maps.h"

namespace map {

struct Paint : public Point {
    Paint(const Point& p, char c) : Point(p), color(c) {}

    char color;
};

using Minerals = std::vector<Paint>;

// extract into choices.h
// ditto
using PrediP = std::function<bool(const Point&)>;
using PlaceP = std::function<char(char, const Point& p)>;

class Continent : public Geology // injection of settings
{
    Minerals minerals;
    MapHolder<char> chrmem{cWater};
    MapHolder<Real> tenmem{1.f};
    MapHolder<int> segments{0};
    unsigned currt_segment = 1u;
    unsigned alloc_seg() { return currt_segment++; }

public:
    const Block visib = bound(0, kMapDim);
    const Block shelf = visib.inset(kShoalz);
    const Point entry = {11, 3};
    const Point ruler = {11, 8};
    const Point magic = {11, 19};
    const Block trail = Block{entry, ruler + Shift{1, 1}};
    const Block major = trail.inset(-7) & visib;

    ChrMap& map = chrmem.map();
    EleMap& echo = tenmem.map();
    IntMap& seg = segments.map();

    std::vector<Point> castle_locs, labels_locs, haven_locs, wonder_locs, enemy_locs;
    std::vector<Point> valued_locs, failed_locs; //transient

    Continent() = default; // TODO inject constants

    // queries

    bool isfirm(int x, int y) const {
        return map[y][x] != cWater;
    }

    bool infirm(int x, int y) const {
        return !isfirm(x, y);
    }

    bool isshore(int x, int y) const {
        return isfirm(x, y) && (infirm(x+1, y) || infirm(x-1, y) || infirm(x, y+1) || infirm(x, y-1));
    }

    bool isshoal(int x, int y) const {
        return infirm(x, y) && (isfirm(x+1, y) || isfirm(x-1, y) || isfirm(x, y+1) || isfirm(x, y-1));
    }

    bool isshore(int x, int y, const Shift& diag) const {
        return isfirm(x, y) && (infirm(x+diag.dx, y) || infirm(x, y+diag.dy));
    }

    bool isplain(int x, int y) const {
        return map[y][x] == cPlain;
    }

    bool onplain(int x, int y) const {
        return isplain(x, y) || isplain(x+1, y) || isplain(x-1, y) || isplain(x, y+1) || isplain(x, y-1);
    }

    // aggregates

    unsigned countSand() const;

    // primitives

    void stroke(const Point& p0, const Point& p1, char color);
    void putCastle(int x, int y, unsigned no);
    void markGates(); // castle gates become '//'s
    void markHome(); // Castle Majorus and Hero's Haven

    // stages: land mass creation

    void tectonics();
    void formLand();
    void segregate();
    void delugify(unsigned eat_shores);

    // stages: landscape creation

    void irrigate(int x, int y);
    void desertify();
    void makeLakes();
    void petrify();

    // stages: power structures (castles and roads)

    void castleize();
    void tunnelize();
    void stoneEcho(); // likely future roads
    void paveRoads();

    // stages: adjustment, also known as wear and tear

    void polish(); // is "czech" a verb too? just curious
    void vandalize(); // barbaric destruction of woods and rocks too complex to display. currently unused (we can do better)

    // FIXME move these lowly primitives away to the backyard!
    bool is_square(int x, int y, int c) const;
    bool is_square(int x, int y) const;
    bool is_locked(int x, int y) const;
    bool is_locked_square(int x, int y) const;
    bool is_hexen WITH_XY const;
    bool is_freesq WITH_XY const;
    static bool is_friendly(int seg, int ours); // inline it...
    bool is_ourssq(int x, int y, int segment) const;
    void mark_sq(int x, int y, int seg_id);

    void segment(char match, char subst);
    void segment(char match); // default value for arg2 is arg1
    void segment();

    // stages: accumulation of wealth, in many ways

    void specials();

    // stages: migration and population exchange, for convenience

    Real cityCost(unsigned i) const;
    Real cityCost() const;
    void citymize();

    // display/overlay

    void maskCities(bool mask = true);

    /**
     * One attempt at map generation: template (in the O-O sense, not the C++ sense) calling stages.
     * If the attempt fails, SHOULD return a status report and a retrial token. This is TBD, however.
     */
    void generate();
};

} // namespace map

#endif