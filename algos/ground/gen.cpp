#include <cstdio>
#include <iostream>
#include <random>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <functional>

#include "aynrand.h"
#include "choices.h"
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

Block nearby(const Point& p) {
    return square(p, 1).inset(-1);
}

Block screen(const Point& p) {
    return square(p, 1).inset(-2);
}

class Continent {
    MapHolder<char> chrmem{cWater};
    MapHolder<Real> ampMap{1.f};
    MapHolder<int> segments{0};
    unsigned currt_segment = 1u;
    unsigned alloc_seg() { return currt_segment++; }

    using Features = std::vector<Paint>;

public:
    const Block visib = bound(0, kMapDim);
    const Block conti = visib.inset(kShoalz);
    const Point entry = {11, 3};
    const Point ruler = {11, 8};
    const Point magic = {11, 19};
    const Block trail = Block{entry, ruler + Shift{1, 1}};
    const Block major = trail.inset(-7) & visib;

    ChrMap& map = chrmem.map();
    IntMap& seg = segments.map();

    std::vector<Point> castle_locs, labels_locs, haven_locs, wonder_locs, enemy_locs;
    std::vector<Point> valued_locs, failed_locs; //transient

    Continent() = default; // TODO inject constants

bool isfirm(unsigned x, unsigned y) const {
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

bool isshore(unsigned x, unsigned y, const Shift& diag) const {
    return isfirm(x, y) && (infirm(x+diag.dx, y) || infirm(x, y+diag.dy));
}

bool isplain(unsigned x, unsigned y) const {
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

std::vector<Paint> tectonics() {
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

    if(kGround != kMature) {
        for(auto itr = features.begin(); itr != features.end(); ) {
            if(major.covers(*itr) && (itr->color != kGround)) {
                itr = features.erase(itr);
            } else {
                ++itr;
            }
        }
        features.push_back({entry, kGround});
        features.push_back({ruler, kGround});
        if(kWizard != kMature) {
            features.push_back({magic, kWizard});
        }
    }

    return features;
}

void formLand() {
    auto features = tectonics();

    struct Allegiance {
        Real part[kColors];

        Allegiance() { reset(); }

        void reset() { std::fill(part, part + kColors, 0.f); }
    };

    conti.visit([&]WITH_XY{
        Allegiance all;
        for(const Paint& f : features) {
            int dx = f.x - x;
            int dy = f.y - y;
            unsigned color = f.color;
            unsigned dcolor = color ? color : kMaxCol + 1;
            Real decay = kDecay + dcolor * (kDPow2 * dcolor + kDPow1); // Horner
            // color *= color & 1u; // more water but fewer islands!
            Real rbf = (dx * dx + dy * dy);
            Real prominence = 1.f; // decay, etc.
            all.part[color] += prominence * expf(- decay * rbf);
        }
        unsigned idx = 0; // guaranteed to be overwritten
        Real max = 0.f;
        Real sum = 0.f;
        for(unsigned color = 0; color <= kMaxCol + 1; ++color) {
            Real pt = all.part[color];
            sum += pt;
            if(all.part[color] > max) {
                max = pt;
                idx = color;
            }
        }
        // torn edge: make winning harder near water
        Real edge_d = std::min(
            std::min(x, kMapMax - x), 
            std::min(y, kMapMax - y)
        );

        Real lns = std::log10(sum + kRoughn) * kSmooth;
        if(kGround != kMature && x <= ruler.x && y == edge_d) {
            // 10-11,3 water; 12-... land
            lns = (x & 2) ? -3 : -2;
        }
        bool edge_cond = (lns + edge_d > kRugged);

        if(max * kWinner > sum && edge_cond) { // majority winner
            map[y][x] = cWater + idx;
        }
    });
}

void segregate() {
    MapHolder<char> chrout{chrmem};
    ChrMap& chm = chrout.map();
    conti.visit([&]WITH_XY{
        char color = chm[y][x];
        auto segregate = [&](unsigned xo, unsigned yo) {
            if(color != cWater) {
                char ocolor = chm[yo][xo];
                if(ocolor != cWater && color != ocolor) {
                    map[y][x] = cWater; // leave <xo, yo>
                }
            }
        };
        segregate(x + 1, y);
        segregate(x + 1, y + 1);
        segregate(x,     y + 1);
    });
}

void delugify(unsigned eat_shores) {
    Block inset = conti;
    Point g_bay = visib.inset(8u).rand();
    for(; eat_shores; --eat_shores) {
        inset.visit([&]WITH_XY {
            Shift diag = Shift{1, 1} - Shift{x > g_bay.x, y > g_bay.y} * 2;
            if(isshore(x, y, diag)) {
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
    bool_xy issand = [&]WITH_XY {
        return x < kMapDim && y < kMapDim && (cSands == map[y][x]);
    };
    with_xy flipxy = [&]WITH_XY {
        map[y][x] = cWater;
    };
    paint8(issand, flipxy)(x, y);
}

void desertify() {
    visib.visit([&]WITH_XY{
        auto& cell = map[y][x];
        cell = cell == cWater ? cSands : cWoods;
    });
}

unsigned countSand() {
    unsigned sands = 0;
    conti.visit([&]WITH_XY {
        sands += map[y][x] == cSands;
    });
    return sands;
}

void makeLakes() {
    delugify(kDoAcid);
    desertify();
    irrigate(kMargin, kMargin); // outer sea
    unsigned sands = countSand();
    unsigned lakes = sands * kNLakes / kMapDim;
    for(unsigned fl = 0; fl < lakes; ++fl) {
        Point rp = conti.rand();
        irrigate(rp.x, rp.y);
    }
    delugify(kDoRain);
}

void petrify() {
    // principal point grid generation; can be extracted
    constexpr Real kDecay = 0.01f;
    std::vector<Point> ppoints;
    constexpr unsigned kGrid = 3;
    Block grid = bound(0, kGrid);
    std::vector<unsigned> fossil;
    std::vector<Real> magnitudes;
    grid.visit([&]WITH_XY {
        Point pp{kMapDim * x / kGrid, kMapDim * y / kGrid};
        ppoints.push_back(pp);
        magnitudes.push_back(std::exp(rnd::zto1()*4));
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
        Real2 dv{s.dy, -s.dx}; // magnetic!
        return dv.normInPlace(kEyeOfTheCyclone);
    };

    // actual petrification
    // second naive plan:
    // - accumulate flux;
    // - normalize flux;
    // - qualifying if: (a) woods, (b) water OR rocks at normalized flux offset, (c) harmless
    // - rinse, repeat

    struct Coral { Point p; Shift dir; };
    std::multimap<Real, Coral> candydates; // sic(k)
    // AND-convolve 2x2 wood patches once I have enough spatial imagination
    // MapHolder<char> sqmem{false};
    // ChrMap& sqm = sqmem.map();
    conti.visit([&]WITH_XY {
        char& c = map[y][x];
        if(c == cWoods) {
            Real2 flux = {};
            Point p{x, y};
            for(unsigned i = 0; i < ppoints.size(); ++i) {
                Real2 wind = dirvec(p, i);
                wind *= factor(p, i);
                flux += wind;
            }
            if(flux.d2() > 0.001f) { // leave alone the undecided
                Real2 unit = flux.normInPlace();
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
        }
    });
    std::list<Coral> appointees; for(const auto& pair : candydates) {
        appointees.push_back(pair.second); // the last turn the first
    }
    while(appointees.size()) {
        Coral coral = appointees.back(); // copy out for consistency and...
        appointees.pop_back();

        // NOTE: the rotational order at (*) and (**) is opposite (to make tiebreaking more fair)
        //       (We can of course say that it's the Coriolis effect. Sorry Terry, no Diskworld.)

        auto tryFlip = [&](const Point& p, const Shift& along, const Shift axxss) { // beware temporary `dir`...
            // affected cells: coral.p +{0, {0,dir.y}, {dir.x,0}, dir}
            // collateral damage: outer 4-side (in the grow direction)
            //
            //     q  r  ?
            //     ?  A  ?
            //  0  ^  B  ?
            //  0  ^  C  ?
            //     s  D  ?
            //     ?  ?  ?

            Shift dir = along + axxss;
            char c0 = at(map, p);
            char cd = at(map, p + dir);
            char ch = at(map, p + along);
            char cv = at(map, p + axxss);
            auto impenetr = [](char c) { return cWoods == c || cRocks == c; };
            bool barriers = impenetr(c0) && impenetr(cd) && impenetr(ch) && impenetr(cv);
            bool haswoods = c0 == cWoods || cd == cWoods || ch == cWoods || cv == cWoods;
            if(barriers && haswoods) {
                Point d = p + dir * 2; // D for Diagonal
                Point b = p + along * 2, a = b - axxss, c = b + axxss;
                Point q = a - dir;
                Point r = a - axxss;
                Point s = d - along;
                auto stillWoods = [&](const Point& p) {
                    return cWoods == at(map, p) && cWoods == at(map, p + dir) && cWoods == at(map, p + along) && cWoods == at(map, p + axxss);
                };
                bool gtg = true;
                gtg &= cWoods != at(map, a) || stillWoods(q) || stillWoods(r) || stillWoods(a);
                gtg &= cWoods != at(map, b) || stillWoods(a) || stillWoods(b);
                gtg &= cWoods != at(map, c) || stillWoods(b) || stillWoods(c);
                gtg &= cWoods != at(map, d) || stillWoods(c) || stillWoods(d) || stillWoods(s);
                if(gtg) {
                    at(map, p) = at(map, p+dir) = at(map, p+along) = at(map, p+axxss) = cRocks;
                }
                return gtg;
            }
            return false;
        };

        const Shift& dir = coral.dir;
        if(dir.dx && dir.dy) { // diagonal
            // we know the corner and he is us
            Shift along = {dir.dx, 0};
            Shift axxss = {0, dir.dy};
            if(tryFlip(coral.p, along, axxss)) {
                coral.p += along;
                if(tryFlip(coral.p, axxss, along)) {
                    appointees.push_back({coral.p + axxss, dir});
                }
            }
        } else { // principal direction
            // we complete coral.dir with coral.dir.left(), then coral.dir.right() (*)
            Shift axxss = dir.left(); // arbitrary
            if(tryFlip(coral.p, dir, axxss)) {
                appointees.push_back({coral.p + dir, dir});
            } // TODO else try turning left or right
        }
    }
}

void polish() {
    // make sure there are no lone trees (every tree is a part of at least one 2x2 woods square)
    // also applies to sands, mountains, though there may or may not be mountains at this point
    conti.visit([&]WITH_XY{
        char color = map[y][x];
        auto isalso = [&](int dx, int dy) {
            return map[y+dy][x+dx] == color
                   && map[y+dy][x] == color
                   && map[y][x+dx] == color;
        };
        if(cWoods == color || cRocks == color || cSands == color) {
            isalso(-1,-1) || isalso(-1,1) || isalso(1,-1) || isalso(1,1) || (map[y][x] = cPlain);
        }
    });
}

void putCastle(int x, int y, unsigned no) {
    map[y][x-1] = cCCWLB;
    map[y+1][x-1] = cCCWLT;
    map[y][x] = cCGate;
    map[y+1][x] = cCRear + no;
    map[y][x+1] = cCCWRB;
    map[y+1][x+1] = cCCWRT;
    map[y-1][x] = cPlain; // etc.etc.etc.
}

void markHaven() {
    switch(kGround) {
        case kMature:
            return;
        case kMidSea:
            map[3][11] = cShaft;
            map[4][11] = cShaft;
            map[5][11] = cShaft;
            map[7][11] = cCGate;
            break;
        default:
            putCastle(11, 7, 0);
            map[8][11] = cCRear; // '0'
            // TODO fix inlets west and east of Hero's Port
            break;
    }
    map[6][11] = cEntry;
    map[3][12] = cPlain; // will be Hero's Haven
    if(kWizard != kMature) {
        map[19][11] = cTribe;
    }
}

void markGates() {
    for(const Point& point : castle_locs) {
        map[point.y-1][point.x] = cEntry;
    }
}

void castleize() {
    // castle placement
    unsigned castles = 0;
    int y = 0;
    int x = 0;
    Real dither = 0;
    auto goodPlaceForCastle = [&]WITH_XY {
        return (kGround == kMature || !major.covers({x, y}))
        && goodPointForCastle(map[y][x]) && goodPointForCastle(map[y][x+1]) && goodPointForCastle(map[y][x-1])
        && goodPointForCastle(map[y+1][x]) && goodPointForCastle(map[y+1][x+1]) && goodPointForCastle(map[y+1][x-1])
        && goodPointForCEntry(map[y-1][x]);
    };
    while(castles < kCastles) {
        Point gate = conti.rand();
        y = gate.y;

        std::vector<int> ltr(kMapDim);
        int d = 0;
        for(int x = 0; x < kMapDim; ++x) { // shadows outer "x"
            ltr[x] = map[y][x] != cWater /*goodPointForCastle(map[y][x])*/ ? ++d : (d = 0);
        }
        // d = 0; // redundant as the edge of the map is always sea
        std::vector<Real> pbs(kMapDim, 0.f);
        Real sum = 0.f;
        for(int x = kMapDim; x; ) { // ditto
            map[y][--x] != cWater /*goodPointForCastle(map[y][--x])*/ ? ++d : (d = 0);
            if(d && ltr[x]) {
                constexpr int kPushInland = 2;
                int l = kPushInland + ltr[x];  // left
                int r = kPushInland + d;       // right
                int a = std::abs(r - l) & ~1u; // asymmetry
                Real prob = std::sqrt(std::sqrt(1.f / (r*r + l*l))) / (1 + a * a);
                for(const Point& point : castle_locs) { // repulsion
                    prob *= 1.f - 25.f / (25u + (point - Point{x, y}).d2());
                    // prob *= (point - Point{x, y}).d2() >= rad2;
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

                castle_locs.push_back({x, y});
                valued_locs.push_back({x, y - 1});
                putCastle(x, y, castle_locs.size());
                ++castles; // if tran commit
            }
        }
    }
}

void tunnelize(EleMap& echo) {
    if(kGround != kMature) {
        valued_locs.push_back({11, 7});
    }

    Real dither = 0.f;
    conti.visit([&]WITH_XY {
        if(cWoods == map[y][x]) { // <implicitly protects castle gates
            Point p{x, y};
            Block scr = screen(p);
            unsigned woods = 0u;
            scr.visit([&] WITH_XY {
                woods += map[y][x] == cWoods;
            });
            Real rating = (at(echo, p) * 2.f + 0.02f * woods) * 0.15f;
            if((dither += rating) >= 1.f) {
                dither -= std::trunc(dither);
                valued_locs.push_back(p);
            }
        }
    });
}

void stoneEcho(EleMap& echo) {
    constexpr Real kSpore = 0.36f;
    constexpr Real kDecay = 0.16f;
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
    auto blend = [](Real mine, Real with) {
        // should propagate incrementally, but not without resistance
        Real blend = 1.f - (1.f - mine) * (1.f - with);
        return mine + (blend - mine) * kDecay;
    };
    auto blur = [&](EleMap& next, const EleMap& prev) {
        conti.visit([&]WITH_XY {
            Real echo = prev[y][x];
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
    EleMap& echo = ampMap.map();
    stoneEcho(echo);
    tunnelize(echo);

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
        bool is_origin_gate = kGround != kMature && li == castle_locs.size();
        bool is_wizard_cell = kGround != kMature && li == castle_locs.size() + 1 && kWizard != kMature;
        auto edgecond = is_castle_gate ? castle_edgecond : wonder_edgecond;
        auto pathterm = is_castle_gate ? castle_pathterm : wonder_pathterm;

        std::vector<Edge> maze;

        bool inland = true;
        if(!is_castle_gate) {
            if(is_wizard_cell) {
                probe = {11, 19};
            }
            if(is_origin_gate) {
                probe = {11, 6};
                maze.push_back({probe, {0, -1}, 3});
                inland = false;
            } else if(onplain(probe.x, probe.y)) {
                inland = false; // equivalent to "continue"
            } else {
                maze.push_back({probe, {0, 0}, 1}); // allow oases
            }
        }

        while(inland) {
            Shift dir;
            unsigned edge; // = rnd::upto(kTrailz<<1);
            unsigned advance = rnd::upto(maze.size() * 3 + 1);
            Real weight;
            if(advance >= maze.size()) {
                auto weigh = [&](unsigned j) {
                    const Shift dir = dirs.at(j);
                    return at(echo, probe+dir) + at(echo, probe+dir+dir);
                };
                unsigned wcount = dirs.size() - maze.empty(); 
                // NOTE: (is_castle_gate && maze.empty()) wb redundant
                // -- non-castle start populates maze (see if() above)
        
                auto pick = rnk::pickWeighed(wcount, weigh);
                weight = pick.weight;
                dir = dirs.at(pick.sel);
            } else {
                const Edge& base = maze[advance];
                auto weigh = [&](unsigned j) {
                    return at(echo, base.probe + base.dir * j);
                };
                unsigned wcount = base.edge + 1;

                // shoot 1.5 times ahead => grow from the end at 33% prob
                unsigned sel = rnk::pickWeighed(wcount, weigh, 1.5f).sel;
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

void vandalize() {
    unsigned outlier = 0;
    unsigned needcor = 0;

    // not strictly necessary, but reduces the amount of needed correction
    conti.visit([&]WITH_XY {
        char c = map[y][x];
        if(cRocks == c || cWoods == c || cSands == c || cWater == c) {
            if(is_hexen(x, y)) {
                ++outlier;
                if(cPlain == map[y-1][x-1]) map[y-1][x-1] = c; else
                if(cPlain == map[y-1][x+1]) map[y-1][x+1] = c; else
                if(cPlain == map[y+1][x-1]) map[y+1][x-1] = c; else
                if(cPlain == map[y+1][x+1]) map[y+1][x+1] = c; else
                ++needcor;
            }
        }
    });

    // reduces 39 => 12, but many corrections are actually avoidable
    fprintf(stderr, "Hexen spots: %u, Unfixed spots: %u\n", outlier, needcor);
}

struct Outliers {};

bool is_square(unsigned x, unsigned y, int c) {
    return map[y+1][x] == c && map[y+1][x+1] == c && map[y][x+1] == c && map[y][x] == c;
};

bool is_square WITH_XY {
    char c = map[y][x];
    return map[y+1][x] == c && map[y+1][x+1] == c && map[y][x+1] == c;
};

bool is_locked WITH_XY {
    char c = map[y][x];
    return (map[y-1][x] != c || map[y-1][x+1] != c)
        && (map[y+2][x] != c || map[y+2][x+1] != c)
        && (map[y][x-1] != c || map[y+1][x-1] != c)
        && (map[y][x+2] != c || map[y+1][x+2] != c);
}

bool is_locked_square WITH_XY {
    return is_square(x, y) && is_locked(x, y);
}

bool is_hexen WITH_XY {
    char c = map[y][x];
    if((map[y-1][x-1] != c && map[y+1][x+1] != c)
    || (map[y-1][x+1] != c && map[y+1][x-1] != c)) {
        unsigned samec = 0u;
        nearby({x, y}).visit([&] WITH_XY {
            samec += map[y][x] == c;
        });
        // 7 cells make up the "eight" shape
        return samec == 7;
    }
    return false;
}

bool is_freesq WITH_XY {
    return !seg[y][x] && !seg[y][x+1] && !seg[y+1][x] && !seg[y+1][x+1];
};

bool is_friendly(int seg, int ours) {
    return !seg || seg == ours;
}

bool is_ourssq(unsigned x, unsigned y, int segment) {
    int lt = seg[y][x];
    int rt = seg[y][x+1];
    int lb = seg[y+1][x];
    int rb = seg[y+1][x+1];
    bool may_advance = !lt || !rt || !lb || !rb;
    bool no_conflict = is_friendly(lt, segment)
                    || is_friendly(rt, segment)
                    || is_friendly(lb, segment)
                    || is_friendly(rb, segment);
    return may_advance && no_conflict;
};

// using mark_xy = std::function<void(unsigned x, unsigned y, unsigned mark)>;

void mark_sq(int x, int y, int seg_id) {
    seg[y][x] = seg[y][x+1] = seg[y+1][x] = seg[y+1][x+1] = seg_id;
}

// void segment(char match, bool_xy test, mark_xy mark) { // TODO extract into here
//     ;
// }

void segment(char match) {
    // TODO extract paintsq(cWoods|cRocks);
    // TODO reuse for aridization (desert).

    // first, detect immobile blocks
    conti.visit([&]WITH_XY {
        if(map[y][x] == match && is_locked_square(x, y) && is_freesq(x, y)) {
            mark_sq(x, y, alloc_seg());
        }
    });
    conti.visit([&]WITH_XY {
        if(is_square(x, y, match) && is_freesq(x, y)) {
            const unsigned seg_id = alloc_seg();
            paint4([&]WITH_XY {
                return conti.covers({x, y}) && is_square(x, y, match) && is_ourssq(x, y, seg_id);
            }, [&]WITH_XY {
                mark_sq(x, y, seg_id);
            })(x, y);
        }
    });
    unsigned outliers = 0;
    conti.visit([&]WITH_XY {
        if(map[y][x] == match && !seg[y][x]) {
            map[y][x] = cPlain;
            outliers++;
        }
    });
    fprintf(stderr, "Outliers [%c]: %u\n", match, outliers);
}

void segment() {
    segment(cWoods);
    segment(cRocks);
    segment(cSands);
    // segment(cWater);
    // for Desertia, segment cPlain and replace with cSands
}

using RankFN = std::function<Real WITH_XY>;
using SweetP = rnk::Ranked<Point>;

SweetP sweepSpots(RankFN rating) {
    rnk::Ranked<Point> sweetspots;
    conti.visit([&]WITH_XY {
        Real sugar = rating(x, y);
        if(sugar > 0.f) {
            sweetspots.insert({sugar, Point{x, y}});
        }
    });
    return sweetspots;
}

using PrediP = std::function<bool(const Point&)>;
using PlaceP = std::function<char(char, const Point& p)>;

bool placeSpots(unsigned count, const SweetP& sweetspots, PlaceP onc, PrediP canc = [](const Point&){ return true; }) {
    auto itr = sweetspots.crbegin();
    for(unsigned i = 0; i < count; ) {
        if(itr == sweetspots.crend()) return false;
        if(canc(itr->second)) {
            at(map, itr->second) = onc(i, itr->second);
            ++i;
        }
        ++itr;
    }
    return true;
}

bool isbarr(char c){ return cWoods == c || cRocks == c; }

bool issand(char c){ return cSands == c; }

bool iswalk(char c){ return cPlain == c; }

bool is_dry(char c){ return isbarr( c ) || cSands == c; }

bool ispass(char c){ return cSands == c || cPlain == c; }

bool ishard(char c){ return isbarr( c ) || cWater == c; }

void specials() {
    EleMap& echo = ampMap.map();

    auto island = [&]WITH_XY {
        bool l = cWater == map[y][x-1];
        bool r = cWater == map[y][x+1];
        bool t = cWater == map[y+1][x];
        bool b = cWater == map[y-1][x];
        return l && r && t && b;
    };

    auto impasse = [&]WITH_XY {
        bool l = ishard(map[y][x-1]);
        bool r = ishard(map[y][x+1]);
        bool t = ishard(map[y+1][x]);
        bool b = ishard(map[y-1][x]);
        return l==r && t==b && l!=b;
    };

    SweetP sign_spots = sweepSpots([&]WITH_XY {
        Point p{x, y};
        if(at(map, p) == cPlain && !impasse(x, y)) {
            unsigned hard = 0;
            screen(p).visit([&]WITH_XY {
                char c = map[y][x];
                hard += ishard(c);
                hard += cPlain == c || cWater == c; // flatitude
            });
            return (rnd::zto1() - .5f) * hard + 25.f * island(x, y);
        }
        return 0.f;
    });
    Real sig2 = (Real) kMapMem / M_PI / castle_locs.size();
    placeSpots(kLabels, sign_spots, [&](unsigned, const Point& p) {
        labels_locs.push_back(p);
        return cLabel;
    }, [&](const Point p) {
        // if we don't do semantic proximity checks, iteration is faster
        for(const Point& sign : labels_locs) {
            if((p - sign).d2() < sig2) return false;
        }
        return true;
    });

    std::vector<Shift> dirs = {{-1,-1}, {-1, 1}, { 1,-1}, { 1, 1}};
    SweetP city_spots = sweepSpots([&]WITH_XY {
        Point p{x, y};
        if(at(map, p) == cPlain && !impasse(x, y)) {
            unsigned pc = 0;
            unsigned ec = 0;
            nearby(p).visit([&]WITH_XY {
                char c = map[y][x];
                pc += c == cPlain;
                ec |= c == cEntry;
            });
            for(const Shift& dir : dirs) {
                char h = at(map, p + Shift{dir.dx, 0});
                char v = at(map, p + Shift{0, dir.dy});
                char d = at(map, p + dir);
                if(cPlain == h || cPlain == v) {
                    if(cWater == d || cWater == h || cWater == v) {
                        return (.5f + 0.1f * pc + rnd::zto1()) * !ec; // TODO mark quadrant...
                    }
                }
            }
        }
        return 0.f;
    });
    // FIXME (wish) favor open space

    MapHolder<char> nations{true};
    ChrMap& nat = nations.map();
    Real rad2 = (Real) kMapMem / M_PI / castle_locs.size();
    if(kGround != kMature) {
        Real maxFactor = city_spots.rbegin()->first;
        city_spots.insert(city_spots.end(), {maxFactor * 2.f, {12, 3}}); // Hero's Port
    }
    placeSpots(castle_locs.size(), city_spots, [&](unsigned, const Point& p) {
        paint4([&]WITH_XY {
            return (Point{x, y} - p).d2() <= rad2 && (cPlain == map[y][x]) && nat[y][x];
        }, [&]WITH_XY {
            nat[y][x] = false;
        })(p.x, p.y);
        haven_locs.push_back(p);
        return cHaven;
    }, [&](const Point& p) {
        return at(nat, p);
    });

    SweetP sweetspots = sweepSpots([&]WITH_XY {
        if(map[y][x] != cPlain) return 0.f;
        if(kGround != kMature && trail.covers({x, y})) return -1.f;
        char l = map[y][x-1], r = map[y][x+1];
        char t = map[y+1][x], b = map[y-1][x];
        int access = ispass(l) + ispass(r) + ispass(t) + ispass(b);
        int desert = issand(l) + issand(r) + issand(t) + issand(b);
        int nocity = cHaven!=l&&cHaven!=r &&cHaven!=b && cHaven!=t;
        int hidden =(is_dry(l)||is_dry(r))&&(is_dry(t)||is_dry(b));
        Real aridity = std::max(-1.7f + desert, 0.f) * 1.3f;
        return hidden * nocity * (4.f - echo[y][x] + aridity - access); // weights break ties
    });

    std::string bag;
    bag.append({cGift1, cGift2});
    bag.append({cPaper, cGlass});
    bag.append({cMetro, cMetro});
    bag.append(kTribes, cTribe);
    bag.append(kAddMes, cAddMe);
    bag.resize(kChests, cChest);
    rnd::shuffle(bag);

    placeSpots(bag.size(), sweetspots, [&](unsigned, const Point& p) {
        wonder_locs.push_back(p);
        return cChest;
    });
    // now enemies, now do-over

    MapHolder<Real> boredom{0.f};
    EleMap& bore = boredom.map();
    // in addition to crowding prevention, we can pre-mark an area as boring
    SweetP enemyspots = sweepSpots([&]WITH_XY {
        if(map[y][x] != cPlain) return 0.f;
        if(kGround != kMature && trail.covers({x, y})) return -1.f;
        const Real kUpperEstim = 4;
        int guards = 0;
        int barrno = 0;
        Real dumb = .0f;
        nearby({x, y}).visit([&]WITH_XY {
            char c = map[y][x];
            barrno += ishard(c); // "hard" includes water
            guards |= cChest == c;
        });
        screen({x, y}).visit([&]WITH_XY {
            char c = map[y][x];
            guards |= cEntry == c || cHaven == c;
            dumb   += (1.f - dumb) * bore[y][x];
        });
        barrno *= (rnd::zto1() < 0.05f); // 1/20 of narrow tunnels
        Real rating = echo[y][x] + guards + 0.15f * barrno;
        bore[y][x] = (1.f / kUpperEstim) * rating;
        return (1.5f - dumb) * rating;
    });
    placeSpots(kIdiots, enemyspots, [&](unsigned, const Point& p){
        enemy_locs.push_back(p);
        return cEnemy;
    });

    placeSpots(bag.size(), sweetspots, [&](unsigned i, const Point& p) {
        (void) p; // wonder_locs.push_back(p); // TODO rpl w/switch
        return bag[i];
    });
}

Real cityCost(unsigned i) {
    return (haven_locs[i] - castle_locs[i]).d();
}

Real cityCost() {
    Real cost = 0.f;
    for(unsigned i = 0; i < kCastles; ++i) {
        cost += cityCost(i);
    }
    return cost;
}

void citymize() {
    // fprintf(stderr, "E Sum(d2)=%f\n", cityCost());
    constexpr unsigned kAtt = kCastles * kCastles;
    for(unsigned a = 0; a < kAtt; ++a) {
        unsigned i = rnd::upto(kCastles);
        unsigned j = rnd::upto(kCastles);
        if(i != j) {
            Real pre = cityCost(i) + cityCost(j);
            Real post = (haven_locs[i] - castle_locs[j]).d() + (haven_locs[j] - castle_locs[i]).d();
            if(post < pre) {
                // NOTE flip castles, not cities, to spare Hero's Haven
                std::swap(castle_locs[i], castle_locs[j]);
            }
        }
    }
    // fprintf(stderr, "X Sum(d2)=%f\n", cityCost());
    for(unsigned i = 0; i < kCastles; ++i) {
        // re-label castles
        const Point& p = castle_locs[i];
        at(map, p + Shift{0, 1}) = cCRear + 1 + i;
    }
}

void maskCities(bool mask = true) {
    for(unsigned i = 0; i < kCastles; ++i) {
        at(map, haven_locs[i]) = mask ? cHaven : cCRear + 1 + i;
    }
}

void generate() {
    formLand();
    segregate();
    makeLakes();
    castleize();
    paveRoads();
    markHaven();
    polish();
    petrify(); // petrification is conservative/transactional => works on a polished surface
    //polish();// +second polish may or may not be needed after a conservative petrification
    segment();
    //
    markGates();
    specials();
    citymize();
}
};

struct GoldenKey {
    struct Burial { Point p; unsigned cindex = 0; unsigned ucount = 0; bool unique = true; };
    Block conti;

    static char convert(char c) {
        if(c >= cCRear && c <= cCRear + kCastles) {
            c = cCRear;
        }
        else switch(c) { // replace w/a LUT?
            case cPaper:
            case cGlass:
            case cGift1:
            case cGift2:
            case cAddMe:
            case cChest:
            case cEnemy:
                c = cPlain;
        }
        return c;
    }

    std::string look(const ChrMap& map, const Point& p) const {
        std::string hash;
        hash.reserve(25);
        screen(p).visit([&]WITH_XY {
            char c = convert(map[y][x]);
            // TODO add options to bury the key under cSands, cLabel or cEntry
            hash.push_back(c); // (25+ovh)*4*64*64 >= 400k... may want to pack
        });
        return hash;
    }

    void consider(const Continent& cont) {
        const auto& map = cont.map;
        conti = cont.conti; // FIXME make static in Continent
        conti.visit([&]WITH_XY {
            spot.p = Point{x, y};
            char c = convert(map[y][x]);
            if(cPlain == c) {
                stats_locs++;
                std::string key = look(map, spot.p);
                auto ins = papermaps.insert({key, spot});
                ins.first->second.ucount++;
                bool& uniq = ins.first->second.unique;
                if(ins.second) {
                    uniq = true;
                    stats_puts++;
                } else {
                    stats_rejc += uniq;
                    uniq = false;
                }
            }
        });
        maps.push_back(&map);
        spot.cindex++;
    }

    Burial select() const {
        if(stats_puts <= stats_rejc) {
            for(const auto& k : papermaps) {
                fprintf(stderr, "%s [%u]\n", k.first.c_str(), k.second.ucount);
            }
            fprintf(stderr, "No unique map locations! %lu=%u?=%u+%u\n", papermaps.size(), stats_locs, stats_puts, stats_rejc);
            // std::abort();
            return spot;
        }
        unsigned attemptcount = 32; // more than enough, but given the fallback is brute force iteration, err on the safe side
        while(attemptcount > 0) {
            unsigned cindex = rnd::upto(spot.cindex);
            const ChrMap& map = *(maps.at(cindex));
            Point p = conti.rand();
            std::string hash = look(map, p);
            auto itr = papermaps.find(hash);
            if(itr != papermaps.cend() && itr->second.unique) {
                return spot = itr->second;
            }
            --attemptcount;
        }
        // brewt force
        unsigned pick = rnd::upto(stats_puts - stats_rejc);
        unsigned item = 0;
        for(auto itr = papermaps.cbegin(); itr != papermaps.cend(); ++itr) {
            if(item == pick) {
                return itr->second;
            }
            item += itr->second.unique;
        }
        fprintf(stderr, "No unique location found! %lu=%u?=%u+%u\n", papermaps.size(), stats_locs, stats_puts, stats_rejc);
        // std::abort();
        return spot;
    }

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

int main(int argc, char** argv) {
    using namespace map;
    (void) argc;
    (void) argv;

    rnd::seed(kSeed);
    Continent cont;
    cont.generate();

    GoldenKey gk;
    gk.consider(cont);
    map::Point k = gk.select().p;

    // replace the following with *stdout << gk
    char c = cPrize;
    char& pl = at(cont.map, k);
    std::swap(c, pl);
    // wrap the following in *stdout << continent
    // cont.maskCities(false);
    *stdout << cont.map;
    // cont.maskCities();
    std::swap(c, pl); // ... gk

    // TODO add map tuning dump

    return 0;
}
