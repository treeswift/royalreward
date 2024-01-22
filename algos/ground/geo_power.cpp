#include "geography.h"

namespace map {

bool goodPointForCastle(char c) {
    return cWoods == c || cRocks == c || cSands == c;
}

bool goodPointForCEntry(char c) {
    return goodPointForCastle(c) || cPlain == c;
}

bool goodPointForTrails(char c) {
    return goodPointForCEntry(c) || cSands == c;
}

void Continent::putCastle(int x, int y, unsigned no) {
    map[y][x-1] = cCCWLB;
    map[y+1][x-1] = cCCWLT;
    map[y][x] = cCGate;
    map[y+1][x] = cCRear + !!no;
    map[y][x+1] = cCCWRB;
    map[y+1][x+1] = cCCWRT;
    map[y-1][x] = cPlain; // etc.etc.etc.
}

void Continent::markHome() {
    switch(kGround) {
        case kMature:
            return;
        case kMidSea:
            map[3][11] = cRafts;
            map[4][11] = cRafts;
            map[5][11] = cRafts;
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

void Continent::markGates() {
    for(const Point& point : forts_locs) {
        map[point.y-1][point.x] = cEntry;
    }
}

void Continent::castleize() {
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
        Point gate = shelf.rand();
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
                for(const Point& point : forts_locs) { // repulsion
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

                forts_locs.push_back({x, y});
                valued_locs.push_back({x, y - 1});
                putCastle(x, y, forts_locs.size());
                ++castles; // if tran commit
            }
        }
    }
}

void Continent::tunnelize() {
    if(kGround != kMature) {
        valued_locs.push_back({11, 7});
    }

    Real dither = 0.f;
    shelf.visit([&]WITH_XY {
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

void Continent::stoneEcho() {
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
        shelf.visit([&]WITH_XY {
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
    shelf.visit([&]WITH_XY {
        Real weight = swap[y][x];
        echo[y][x] = weight * weight;
    });
}

void Continent::paveRoads() {
    stoneEcho();
    tunnelize();

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
        bool is_castle_gate = li < forts_locs.size();
        bool is_origin_gate = kGround != kMature && li == forts_locs.size();
        bool is_wizard_cell = kGround != kMature && li == forts_locs.size() + 1 && kWizard != kMature;
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

} // namespace map