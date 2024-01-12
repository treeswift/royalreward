#include <cstdio>
#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
#include <functional>

#include "aynrand.h"
#include "maps.h"

namespace map{

/**
 * Generate a map.
 */

struct Paint : public Point {
    Paint(const Point& p, char c) : Point(p), color(c) {}

    char color;
};

struct Allegiance {
    float part[kColors];

    Allegiance() { reset(); }

    void reset() { std::fill(part, part + kColors, 0.f); }
};

bool goodPointForCastle(char c) {
    return cWoods == c || cRocks == c || cSands == c;
}

bool goodPointForCEntry(char c) {
    return goodPointForCastle(c) || cPlain == c;
}

bool goodPointForTrails(char c) {
    return goodPointForCEntry(c) || cSands == c;
}

class Continent {
    Block visib = bound(0, kMapDim);
    Block conti = visib.inset(kShoalz);
    MapHolder<char> chrmem{cWater};
    std::vector<Point> castle_locs;

    using Features = std::vector<Paint>;

public:
    Continent() = default; // TODO inject constants

    ChrMap& map() { return chrmem.map(); }
    const ChrMap& map() const { return chrmem.map(); }

void stroke(Features& features, const Point& p0, const Point& p1, char color) {
    features.push_back({p0, color});

    for(Real f = 0.f; f < 1.f; f += (1.f / kMapDim)) {
        Point p = p0.blend(p1, f);
        (p == features.back()) || (features.push_back({p, color}), true);
    }
}

std::vector<Paint> minerals() {
    std::vector<Paint> features;

    Block sbox = bound(kMinBox, kMinBox + kFeaAmp);
    Point base = corner(0);
    Shift dmax = diag(kMapDim - kMinBox);

    for(unsigned fl = 0; fl < kNLines; ++fl) {
        Shift s = sbox.rand() - base;
        Point d = base + (dmax - s).rand();

        Point p0 = d + s.rand();
        Point p1 = d + s.rand();

        char color = rnd::upto(kMaxCol) + 1u;
        stroke(features, p0, p1, color);
    }
    return features;
}

void formLand() {
    auto features = minerals();
    ChrMap& map = chrmem.map();

    conti.visit([&]WITH_XY{
        Allegiance all;
        for(const Paint& f : features) {
            int dx = f.x - x;
            int dy = f.y - y;
            Real decay = kDecay + f.color * (kDPow2 * f.color + kDPow1); // Horner
            unsigned color = f.color;
            // color *= color & 1u; // more water but fewer islands!
            float rbf = (dx * dx + dy * dy);
            float prominence = 1.f; // decay, etc.
            all.part[color] += prominence * expf(- decay * rbf);
        }
        unsigned idx = 0; // guaranteed to be overwritten
        float max = 0.f;
        float sum = 0.f;
        for(unsigned color = 0; color <= kMaxCol; ++color) {
            float pt = all.part[color];
            sum += pt;
            if(all.part[color] > max) {
                max = pt;
                idx = color + 1u;
            }
        }
        // torn edge: make winning harder near water
        float edge_d = std::min(
            std::min(x, kMapMax - x), 
            std::min(y, kMapMax - y)
        );

        float lns = std::log10(sum + kRoughn) * kSmooth;

        if(max * kWinner > sum && (lns + edge_d > kRugged)) { // majority winner
            map[y][x] = cWater + idx;
        }
    });
}

void segregate() {
    ChrMap& map = this->map();
    MapHolder<char> chrout{chrmem};
    ChrMap& chm = chrout.map();
    conti.visit([&]WITH_XY{
        char color = chm[y][x];
        auto segregate = [&](unsigned xo, unsigned yo) {
            if(color != cWater) {
                char ocolor = chm[yo][xo];
                if(ocolor != cWater && color != ocolor) {
                    map[y][x] = map[yo][xo] = cWater;
                }
            }
        };
        segregate(x + 1, y);
        segregate(x + 1, y + 1);
        segregate(x,     y + 1);
    });
}

void desertify() {
    ChrMap& map = this->map();
    // desertify
    for_rect(0, 0, kMapDim, kMapDim, [&]WITH_XY{
        auto& cell = map[y][x];
        cell = cell == cWater ? cSands : cWoods;
    });
    bool_xy issand = [&]WITH_XY {
        return x < kMapDim && y < kMapDim && (cSands == map[y][x]);
    };
    with_xy flipxy = [&]WITH_XY {
        map[y][x] = cWater;
    };
    with_xy irrigate = paint8(issand, flipxy);

    // MOREINFO: reasons for controlled irrigation? (the Bridge spell fixes most of the shortcomings, really)
    irrigate(kMargin, kMargin);
    for(unsigned fl = 0; fl < kNLakes; ++fl) {
        Point rp = conti.rand();
        irrigate(rp.x, rp.y);
    }
}

void polish() {
    // make sure there are no lone trees (every tree is a part of at least one 2x2 woods square)
    // also applies to mountains, though there may or may not be mountains at this point
    ChrMap& map = this->map();
    conti.visit([&]WITH_XY{
        char color = map[y][x];
        auto isalso = [&](int dx, int dy) {
            return map[y+dy][x+dx] == color
                   && map[y+dy][x] == color
                   && map[y][x+dx] == color;
        };
        if(cWoods == color || cRocks == color) { // MOREINFO: are 1x1 patches of desert allowed?
            isalso(-1,-1) || isalso(-1,1) || isalso(1,-1) || isalso(1,1) || (map[y][x] = cPlain);
        }
    });
}

void castleize() {
    // castle placement
    ChrMap& map = this->map();
    constexpr unsigned kCastles = 9;
    unsigned castles = 0;
    unsigned y = 0;
    unsigned x = 0;
    Real dither = 0;
    auto goodPlaceForCastle = [&]WITH_XY {
        return goodPointForCastle(map[y][x]) && goodPointForCastle(map[y][x+1]) && goodPointForCastle(map[y][x-1])
        && goodPointForCastle(map[y+1][x]) && goodPointForCastle(map[y+1][x+1]) && goodPointForCastle(map[y+1][x-1])
        && goodPointForCEntry(map[y-1][x]);
    };
    while(castles < kCastles) {
        Point gate = conti.rand();
        y = gate.y;

        std::vector<int> ltr(kMapDim);
        int d = 0;
        for(unsigned x = 0; x < kMapDim; ++x) { // shadows outer "x"
            ltr[x] = goodPointForCastle(map[y][x]) ? ++d : (d = 0);
        }
        // d = 0; // redundant as the edge of the map is always sea
        std::vector<Real> pbs(kMapDim, 0.f);
        Real sum = 0.f;
        for(unsigned x = kMapDim; x; ) { // ditto
            goodPointForCastle(map[y][--x]) ? ++d : (d = 0);
            if(d && ltr[x]) {
                unsigned dst = d - ltr[x];
                Real prob = std::sqrt(1.f / d / ltr[x]) / (1 + dst * dst);
                for(const Point& point : castle_locs) { // repulsion
                    prob *= (1.f - 1.f / (point - Point{x, y}).d2());
                }
                sum += (pbs[x] = prob);
            }
        }
        if(sum < 1e-7f) // epsilon
            continue;

        while((dither += pbs[x]) < 1.) {
            ++x; x%=kMapDim;
        }
        while(dither >= 1.f) {
            dither -= 1.f;
        }

        if(goodPointForCastle(map[y][x]) && goodPointForCastle(map[y+1][x])) {
            // tight placement adjustment
            if(!(goodPointForCastle(map[y][x-1]) && goodPointForCastle(map[y+1][x-1]))) {
                ++x;
            } else if(!(goodPointForCastle(map[y][x+1]) && goodPointForCastle(map[y+1][x+1]))) {
                --x;
            }
            if(goodPlaceForCastle(x, y)) {
                unsigned resistance = 15; // TODO softcode
                // possibly promote the castle north(view coordinates) [=south(map coordinates)]
                while(rnd::upto(100) > resistance && goodPointForCastle(map[y+2][x-1]) &&
                    goodPointForCastle(map[y+2][x]) && goodPointForCastle(map[y+2][x+1])) {
                        ++y;
                        resistance += 20;
                    }

                // FIXME make transactional!
                map[y][x-1] = '[';
                map[y+1][x-1] = '{';
                map[y][x] = cCGate;
                map[y+1][x] = cCRear + castles;
                map[y][x+1] = ']';
                map[y+1][x+1] = '}';
                map[y-1][x] = cPlain; // etc.etc.etc.
                castle_locs.push_back({x, y});
                ++castles; // if tran commit
            }
        }
    }
}

void paveRoads() {
    // trails
    struct Edge {
        Point probe;
        Shift dir;
        unsigned edge;
    };
    ChrMap& map = this->map();
    constexpr unsigned kTrailz = 11;
    for(const Point& cgate : castle_locs) {

        std::vector<Edge> maze;

        bool inland = true;
        while(inland) {
            Point probe = cgate - Shift{0, 1};
            Shift dir;
            unsigned edge = rnd::upto(kTrailz<<1);
            unsigned advance = rnd::upto(maze.size() * 3 + 1);
            if(advance >= maze.size()) {
                int dx = (edge & 1);
                int sg = (edge & 2) - 1;
                dir = Shift{dx, 1 - dx} * sg;
            } else {
                const Edge& base = maze[advance];
                probe = base.probe + base.dir * rnd::upto(base.edge + 1);
                dir = (edge & 1) ? base.dir.left() : base.dir.right();
            }
            edge >>= 1;

            bool canExtend = true;
            Point start = probe;
            unsigned i = 0;
            for(; (i < edge) && canExtend; ++i) {
                probe+=dir;
                canExtend &= goodPointForTrails(map[probe.y][probe.x]);
                // TODO consider penalty for self-intersection
            }
            if(map[probe.y][probe.x] == cWater) {
                canExtend = true;
                edge = i;
                inland = false;
            }
            if(canExtend) {
                maze.push_back({start, dir, edge});
            }
        }
        for(const Edge& edge : maze) {
            Point trail = edge.probe;
            for(unsigned i = 1; i < edge.edge; ++i) {
                trail += edge.dir;
                char& c = map[trail.y][trail.x];
                if(c != cSands) c = cPlain;
            }
        }
    }
}

void generate() {
    formLand();
    segregate();
    desertify();
    castleize();
    paveRoads();

    polish();
}
};

} // namespace map

int main(int argc, char** argv) {
    using namespace map;

    rnd::seed(kSeed);
    Continent cont;
    cont.generate();
    *stdout << cont.map();

    return 0;
}
