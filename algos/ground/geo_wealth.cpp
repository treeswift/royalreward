#include "geography.h"

namespace map {

bool isbarr(char c){ return cWoods == c || cRocks == c; }

bool issand(char c){ return cSands == c; }

bool iswalk(char c){ return cPlain == c; }

bool is_dry(char c){ return isbarr( c ) || cSands == c; }

bool ispass(char c){ return cSands == c || cPlain == c; }

bool ishard(char c){ return isbarr( c ) || cWater == c; }

void Continent::specials() {
    using namespace rnk;
    auto sweepSpots = [&](RankFN rating) { return sweetSpots(shelf, rating); };
    auto placeSpots = [&](unsigned count, const SweetP& sweetspots, PlaceP onc, PrediP canc = [](const Point&){ return true; }) {
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
    };

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

Real Continent::cityCost(unsigned i) const {
    return (haven_locs[i] - castle_locs[i]).d();
}

Real Continent::cityCost() const {
    Real cost = 0.f;
    for(unsigned i = 0; i < kCastles; ++i) {
        cost += cityCost(i);
    }
    return cost;
}

void Continent::citymize() {
    // fprintf(stderr, "E Sum(d2)=%f\n", cityCost());
    const unsigned kAtt = kCastles * kCastles;
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

void Continent::maskCities(bool mask) {
    for(unsigned i = 0; i < kCastles; ++i) {
        at(map, haven_locs[i]) = mask ? cHaven : cCRear + 1 + i;
    }
}

} // namespace map
