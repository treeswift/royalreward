#include <cstdio>
#include <iostream>
#include <random>
#include <vector>
#include <list>
#include <map>
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
    std::vector<Point> castle_locs, wonder_locs, valued_locs, failed_locs;

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

bool onplain(unsigned x, unsigned y) const {
    return isplain(x, y) || isplain(x+1, y) || isplain(x-1, y) || isplain(x, y+1) || isplain(x, y-1);
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

void petrify() {
    // principal point grid generation; can be extracted
    constexpr Real kDecay = 0.05f;
    std::vector<Point> ppoints;
    constexpr unsigned kGrid = 3;
    Block grid = bound(0, kGrid);
    std::vector<unsigned> fossil;
    std::vector<Real> magnitudes;
    grid.visit([&]WITH_XY {
        Point pp{kMapDim * x / kGrid, kMapDim * y / kGrid};
        ppoints.push_back(pp);
        magnitudes.push_back(std::exp(rnd::upto(4)));
        fossil.push_back((rnd::upto(255) & 128) >> 7);
    });
    auto factor = [&](const Point& p, unsigned ppi) {
        // MOREINFO use simple inverse square? ("1+s.d2")?
        constexpr bool kDiversity = false;
        Real power = -kDecay * (p - ppoints.at(ppi)).d2();
        if(kDiversity) power *= magnitudes.at(ppi);
        Real decay = std::exp(power);
        if(kDiversity) decay *= magnitudes.at(ppi);
        return decay;
    };
    
    // Flux. MOREINFO make Shift a template class instead?
    struct Real2 {
        Real x, y;
        Real2& operator+=(const Real2& other) { x += other.x; y += other.y; return *this; }
        Real2& operator*=(const Real& factor) { x *= factor; y *= factor; return *this; }
        // TOOD norm(), safenorm()
        Real d2() const { return x*x + y*y; }
        Real d(Real eoc = 0.f) const { return std::sqrt(eoc + d2()); }
        Real2& normInPlace(Real eoc = 0.f)  { Real den = 1.f / d(eoc); return (*this)*=den; }
        Real2 norm(Real eoc = 0.f) const { auto copy = *this; return copy.normInPlace(eoc); }
    };
    constexpr Real kEyeOfTheCyclone = 1.f;
    auto dirvec = [&](const Point& p, unsigned ppi) {
        Shift s = ppoints.at(ppi) - p;
        Real2 dv{s.dx, s.dy};
        return dv.normInPlace(kEyeOfTheCyclone);
    };

    // actual petrification
    // second naive plan:
    // - accumulate flux;
    // - normalize flux;
    // - qualifying if: (a) woods, (b) water OR rocks at normalized flux offset, (c) harmless
    // - rinse, repeat

    struct Coral { Point p; Shift dir; };
    ChrMap& map = this->map();
    std::multimap<Real, Coral> candydates; // sic(k)
    conti.visit([&]WITH_XY {
        char& c = map[y][x];
        if(c == cWoods) {
            Real2 flux;
            Point p{x, y};
            for(unsigned i = 0; i < ppoints.size(); ++i) {
                Real2 wind = dirvec(p, i);
                wind *= factor(p, i);
                flux += wind;
            }
            Real2 unit = flux.normInPlace(kEyeOfTheCyclone);
            unit *= 2.f; // 0.5 becomes 1
            Shift offset = {std::truncf(unit.x), std::truncf(unit.y)};
            if(offset.dx || offset.dy) {
                Point refpt = p + offset;
                char c = at(map, refpt);
                if(cWater == c) {
                    candydates.insert({flux.d2(), {p, -offset}});
                }
            }
        }
    });
    std::list<Coral> appointees; for(const auto& pair : candydates) {
        appointees.push_back(pair.second); // the last turn the first
    }
    while(appointees.size()) {
        Coral coral = appointees.back(); // copy out for consistency and...
        appointees.pop_back();

        // NOTE: the rotational order at (*) and (**) is opposite (to make tiebreaking more fair)
        //      (We can of course say that it's the Coriolis force. Sorry Terry, no Diskworld...)

        // lambda tryFlip = ...;

        if(coral.dir.dx && coral.dir.dy) { // diagonal
            // we know the corner and he is us
            // affected cells: coral.p +{0, {0,dir.y}, {dir.x,0}, dir}
            // collateral damage: outer 4x4 corner
            //
            //       ?  ?  ?
            //       ?  *  ?
            //    0  ^  *  ?
            //    ^  ^  *  ?
            // ?  *  *  *  ?
            // ?  ?  ?  ?  ?

            // we replace the 2x2 wood with 2x2 rock... (**)
            // ...pushing left-side|right-side|diag as possible successors

            // if(tryFlip(...)) continue;

        } else { // principal direction
            // we complete coral.dir with coral.dir.left(), then coral.dir.right() (*)

            // we replace the 2x2 wood with 2x2 rock... (**)
            // ...pushing same-side|right-diag|left-diag as possible successors
        }
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

void markGates() {
    ChrMap& map = this->map();
    // for(const Point& point : wonder_locs) {
    //     map[point.y][point.x] = cChest; // DBG: not real chests, more like forest clearings
    // }
    // for(const Point& point : failed_locs) {
    //     map[point.y][point.x] = cLabel; // DBG: rejected nooks and secret places
    // }
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

void tunnelize(EleMap& echo) {
    ChrMap& map = this->map();
    Real dither = 0.f;
    conti.visit([&]WITH_XY {
        if(cWoods == map[y][x]) { // <implicitly protects castle gates
            Point p{x, y};
            Block screen = square(p - Shift{2, 2}, 5); // TODO extract
            unsigned woods = 0u;
            screen.visit([&] WITH_XY {
                woods += map[y][x] == cWoods;
            });
            Real rating = (at(echo, p) * 2.f + 0.02f * woods) * 0.15f;
            if((dither += rating) >= 1.f) {
                dither -= std::trunc(dither);
                wonder_locs.push_back(p);
                valued_locs.push_back(p);
            }
        }
    });
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
    MapHolder<Real> swapHolder{0.f};
    EleMap& swap = swapHolder.map();
    visib.inset(1).visit([&]WITH_XY {
        swap[y][x] = isshoal(x, y) ? coast(x, y) : (1e-4f * (unsigned) map[y][x]);
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
    for(unsigned i = 0; i < kEchoes; i += 2) {
        blur(echo, swap);
        blur(swap, echo);
    }
    conti.visit([&]WITH_XY {
        Real weight = swap[y][x];
        echo[y][x] = weight * weight;
    });
}

void paveRoads() {
    ChrMap& map = this->map();
    MapHolder<Real> ampMap{1.f};
    EleMap& echo = ampMap.map();
    stoneEcho(echo);
    tunnelize(echo);
    dbgEcho(echo);

    // trails
    struct Edge {
        Point probe;
        Shift dir;
        unsigned edge;
    };
    constexpr unsigned kTrailz = 41;
    constexpr unsigned kMaxTrl = 7;
    // TODO extract predicate type
    std::function<bool(const Point&)> castle_edgecond = [&](const Point& probe) {
        return goodPointForTrails(map[probe.y][probe.x]);
    };
    std::function<bool(const Point&)> wonder_edgecond = [&](const Point& probe) {
        return castle_edgecond(probe) && !isplain(probe.x, probe.y);
    };
    std::function<bool(const Point&)> castle_pathterm = [&](const Point& probe) {
        return infirm(probe.x, probe.y) || isshore(probe.x, probe.y);
    };
    std::function<bool(const Point&)> wonder_pathterm = [&](const Point& probe) {
        return castle_pathterm(probe) || onplain(probe.x, probe.y);
    };
    std::vector<Shift> dirs = {{-1,0},{1,0},{0,-1},{0,1}};
    for(unsigned li = 0; li < valued_locs.size(); ++li) {
        // we want the element modifiable + want its index
        // ...or extract loop body and apply to both vecs?
        Point probe = valued_locs[li];
        bool is_castle_gate = li < castle_locs.size();
        auto edgecond = is_castle_gate ? castle_edgecond : wonder_edgecond;
        auto pathterm = is_castle_gate ? castle_pathterm : wonder_pathterm;

        std::vector<Edge> maze;

        bool inland = true;
        if(!is_castle_gate) {
            if(onplain(probe.x, probe.y)) {
                inland = false; // equivalent to "continue"
            }
            maze.push_back({probe, {0, 0}, 1}); // allow oases
        }

        while(inland) {
            Shift dir;
            unsigned edge; // = rnd::upto(kTrailz<<1);
            unsigned advance = rnd::upto(maze.size() * 3 + 1);
            Real weight;
            if(advance >= maze.size()) {
                // int dx = (edge & 1);
                // int sg = (edge & 2) - 1;
                // dir = Shift{dx, 1 - dx} * sg;
                std::vector<Real> weights;
                Real total_weight = 0.f;
                unsigned wcount = dirs.size() - is_castle_gate;
                for(unsigned j = 0; j < wcount; ++j) {
                    const Shift dir = dirs.at(j);
                    Real weight = at(echo, probe+dir) + at(echo, probe+dir+dir);
                    weights.push_back(weight);
                    total_weight += weight;
                }
                float cast = rnd::zto1() * total_weight;
                unsigned sel = wcount - 1;
                for(unsigned j = 0; j < wcount; ++j) {
                    if((cast -= weights[j]) <= 0.f) {
                        sel = j;
                        break;
                    }
                }
                dir = dirs.at(sel);
                weight = weights.at(sel);
            } else {
                // probe = base.probe + base.dir * rnd::upto(base.edge + 1);
                // dir = (edge & 1) ? base.dir.left() : base.dir.right();
                const Edge& base = maze[advance];
                Point retro = base.probe; // replace with multiplication
                std::vector<Real> weights;
                Real total_weight = 0.f;
                for(unsigned j = 0; j <= base.edge; ++j) {
                    Real weight = at(echo, retro);
                    weights.push_back(weight);
                    total_weight += weight;
                    retro += base.dir;
                }
                float cast = rnd::zto1() * total_weight * 1.5f; // grow from end: 33% 
                unsigned sel = base.edge;
                for(unsigned j = 0; j <= base.edge; ++j) {
                    if((cast -= weights[j]) <= 0.f) {
                        sel = j;
                        break;
                    }
                }
                probe = base.probe + base.dir * sel;
                Shift ldir = base.dir.left();
                Shift rdir = base.dir.right();
                Real lweight = at(echo, probe + ldir) + at(echo, probe + ldir + ldir);
                Real rweight = at(echo, probe + rdir) + at(echo, probe + rdir + rdir);
                if(lweight > rweight) { // marginal rotational asymmetry...
                    weight = lweight;
                    dir = ldir;
                } else {
                    weight = rweight;
                    dir = rdir;
                }
                if(sel == base.edge && rnd::zto1() < 0.2f) { // ...or we can continue
                    Real fweight = at(echo, probe + base.dir) + at(echo, probe + base.dir + base.dir);
                    if(fweight > weight) {
                        weight = fweight;
                        dir = base.dir;
                    }
                }
            }
            // edge >>= 1;
            edge = std::min(1u + (unsigned) (kTrailz * weight), kMaxTrl); // rnd::upto(kTrailz*weight)
            // if(edge > 5) edge -= (edge % 3);     // "snap to grid"

            bool canExtend = true;
            Point start = probe;
            unsigned i = 0;
            for(; (i < edge) && (canExtend &= edgecond(probe + dir)); ++i) {
                probe += dir;
            }
            if(pathterm(probe += dir)) {
                canExtend = true;
                // phobia of self-intersection (TODO softcode constants!!)
                if(i >= 5 && !is_castle_gate && rnd::zto1() < 0.3f) {
                    i -= 3;
                } else {
                    inland = false;
                }
                edge = i;
            }
            if(canExtend) {
                maze.push_back({start, dir, edge});
                probe = start + dir * edge; // advance
            } else {
                probe = start; // go back
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
    paveRoads();
    makeLakes();
    polish();
    petrify(); // petrification is conservative/transactional => works on a polished surface
    polish();  // +second polish may or may not be needed after a conservative petrification
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
