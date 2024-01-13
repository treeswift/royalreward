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
    MapHolder<char> ovrmem{cWater};
    std::vector<Point> castle_locs, wonder_locs, valued_locs;

    using Features = std::vector<Paint>;

public:
    Continent() = default; // TODO inject constants

    ChrMap& map() { return chrmem.map(); }
    const ChrMap& map() const { return chrmem.map(); }

bool isfirm(unsigned x, unsigned y) const {
    const ChrMap& map = this->map();
    return map[y][x] != cWater;
}

bool infirm(unsigned x, unsigned y) const {
    return !isfirm(x, y);
}

bool isshore(unsigned x, unsigned y) const {
    return isfirm(x, y) && (infirm(x+1, y) || infirm(x-1, y) || infirm(x, y+1) || infirm(x, y-1));
}

bool isshoal(unsigned x, unsigned y) const {
    return infirm(x, y) && (isfirm(x+1, y) || isfirm(x-1, y) || isfirm(x, y+1) || isfirm(x, y-1));
}

bool isplain(unsigned x, unsigned y) const {
    const ChrMap& map = this->map();
    return map[y][x] == cPlain;
}

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

    struct Allegiance {
        float part[kColors];

        Allegiance() { reset(); }

        void reset() { std::fill(part, part + kColors, 0.f); }
    };

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

void delugify(unsigned eat_shores) {
    ChrMap& map = this->map();
    Block inset = conti.inset(2u);
    for(; eat_shores; --eat_shores) {
        inset.visit([&]WITH_XY {
            if(isshore(x, y)) {
                map[y][x] = cMagma;
            }
        });
        inset.visit([&]WITH_XY {
            if(cMagma == map[y][x]) {
                map[y][x] = cWater;
            }
        });
    }
}

void irrigate(unsigned x, unsigned y) {
    ChrMap& map = this->map();
    bool_xy issand = [&]WITH_XY {
        return x < kMapDim && y < kMapDim && (cSands == map[y][x]);
    };
    with_xy flipxy = [&]WITH_XY {
        map[y][x] = cWater;
    };
    paint8(issand, flipxy)(x, y);
}

void desertify() {
    ChrMap& map = this->map();
    // desertify
    visib.visit([&]WITH_XY{
        auto& cell = map[y][x];
        cell = cell == cWater ? cSands : cWoods;
    });
    // MOREINFO: reasons for controlled irrigation? (the Bridge spell fixes most of the shortcomings, really)
    irrigate(kMargin, kMargin);
}

void makeLakes() {
    delugify(kDoAcid);
    for(unsigned fl = 0; fl < kNLakes; ++fl) {
        Point rp = conti.rand();
        irrigate(rp.x, rp.y);
    }
    delugify(kDoRain);
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

void markGates() {
    ChrMap& map = this->map();
    for(const Point& point : wonder_locs) {
        map[point.y][point.x] = cChest;
    }
    for(const Point& point : castle_locs) {
        map[point.y-1][point.x] = cEntry;
    }
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
                constexpr unsigned kPushInland = 2u;
                int l = kPushInland + ltr[x];  // left
                int r = kPushInland + d;       // right
                int a = std::abs(r - l) & ~1u; // asymmetry
                Real prob = std::sqrt(std::sqrt(1.f / (r*r + l*l))) / (1 + a * a);
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
                valued_locs.push_back({x, y - 1});
                ++castles; // if tran commit
            }
        }
    }
}

void tunnelize() {
    ChrMap& map = this->map();
    constexpr unsigned kChambers = 95;
    for(unsigned i = 0; i < kChambers; ++i) {
        Point p = conti.rand();
        char c = map[p.y][p.x];
        if(cWoods == c) { // <== also implicitly protects castle gates
            unsigned woods = 0u;
            Block screen = square(p - Shift{2, 2}, 5); // TODO extract
            screen.visit([&] WITH_XY {
                woods += map[y][x] == cWoods;
            });
            if(rnd::inrg(13, 25) < woods) {
                wonder_locs.push_back(p);
                valued_locs.push_back(p);
            }
        }
    }
}

void stoneEcho(EleMap& echo) {
    ChrMap& map = this->map();
    constexpr float kSpore = 0.36f;
    constexpr float kDecay = 0.16f;
    auto coast = [&]WITH_XY {
        int grady = infirm(x-1, y+1) + infirm(x,y+1) + infirm(x+1,y+1) -
                   (isfirm(x-1, y-1) + infirm(x,y-1) + infirm(x+1,y-1));
        int gradx = infirm(x+1, y-1) + infirm(x+1,y) + infirm(x+1,y+1) -
                   (isfirm(x-1, y-1) + infirm(x-1,y) + infirm(x-1,y+1));
        return kSpore * (3.f + std::abs(grady * gradx)) / 12.f;
    };
    visib.inset(1).visit([&]WITH_XY {
        echo[y][x] = isshoal(x, y) ? coast(x, y) : (1e-4f * (unsigned) map[y][x]);
    });
    auto blend = [](float mine, float with) {
        // should propagate incrementally, but not without resistance
        float blend = 1.f - (1.f - mine) * (1.f - with);
        return mine + (blend - mine) * kDecay;
    };
    auto blur = [&](EleMap& next, const EleMap& prev) {
        conti.visit([&]WITH_XY {
            float echo = prev[y][x];
            if(isfirm(x+1, y) && isfirm(x+2, y)) echo = blend(echo, prev[y][x+3]);
            if(isfirm(x-1, y) && isfirm(x-2, y)) echo = blend(echo, prev[y][x-3]);
            if(isfirm(x, y+1) && isfirm(x, y+2)) echo = blend(echo, prev[y+3][x]);
            if(isfirm(x, y-1) && isfirm(x, y-2)) echo = blend(echo, prev[y-3][x]);
            next[y][x] = echo;
        });
    };
    MapHolder<Real> swapHolder{0.f};
    EleMap& swap = swapHolder.map();
    for(unsigned i = 0; i < kEchoes; i += 2) {
        blur(swap, echo);
        blur(echo, swap);
    }
}

void paveRoads() {
    ChrMap& map = this->map();
    MapHolder<Real> ampMap{1.f};
    EleMap& echo = ampMap.map();
    stoneEcho(echo);
    dbgEcho(echo);

    // trails
    struct Edge {
        Point probe;
        Shift dir;
        unsigned edge;
    };
    constexpr unsigned kTrailz = 11;
    // TODO extract predicate type
    std::function<bool(const Point&)> castle_edgeterm = [&](const Point& probe) {
        return goodPointForTrails(map[probe.y][probe.x]);
    };
    std::function<bool(const Point&)> wonder_edgeterm = [&](const Point& probe) {
        return castle_edgeterm(probe) && !isplain(probe.x, probe.y);
    };
    std::function<bool(const Point&)> castle_pathterm = [&](const Point& probe) {
        return infirm(probe.x, probe.y) || isshore(probe.x, probe.y);
    };
    std::function<bool(const Point&)> wonder_pathterm = [&](const Point& probe) {
        return castle_pathterm(probe) || isplain(probe.x, probe.y);
    };

    for(unsigned li = 0; li < valued_locs.size(); ++li) {
        // we want the element modifiable + want its index
        // ...or extract loop body and apply to both vecs?
        Point probe = valued_locs[li];
        bool is_castle_gate = li < castle_locs.size();
        auto edgeterm = is_castle_gate ? castle_edgeterm : wonder_edgeterm;
        auto pathterm = is_castle_gate ? castle_pathterm : wonder_pathterm;

        std::vector<Edge> maze;

        bool inland = true;
        while(inland) {
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
            for(; (i < edge) && (canExtend &= edgeterm(probe + dir)); ++i) {
                probe += dir;
                // TODO consider penalty for self-intersection
            }
            if(!canExtend && pathterm(probe + dir)) {
                canExtend = true;
                edge = i;
                inland = false;
            }
            if(canExtend) {
                maze.push_back({start, dir, edge});
                probe = start + dir * edge; // ensure
            } else {
                probe = start;
            }
        }
        for(const Edge& edge : maze) {
            Point trail = edge.probe;
            for(unsigned i = 1; i <= edge.edge; ++i) {
                trail += edge.dir;
                char& c = map[trail.y][trail.x];
                if(c != cSands) c = cPlain;
            }
        }
    }
}

void dbgEcho(const EleMap& echo) {
    ChrMap& map = ovrmem.map();
    conti.visit([&] WITH_XY {
        if(map[y][x] == cWoods) {
            float amp = echo[y][x];
            // gamma correction:
            amp = 1.f - amp;
            amp = amp * amp;
            amp = 1.f - amp;
            // end of correction
            map[y][x] = '0' + 10.f * amp;
        }
    });
}

void generate() {
    formLand();
    segregate();
    desertify();
    castleize();
    tunnelize();
    paveRoads();
    makeLakes();

    polish();
    markGates();
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
