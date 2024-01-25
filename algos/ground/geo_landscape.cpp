#include "geography.h"

#include <list>

namespace map {

void Continent::delugify(unsigned eat_shores) {
    Block inset = shelf;
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

void Continent::irrigate(int x, int y) {
    bool_xy issand = [&]WITH_XY {
        return x < kMapDim && y < kMapDim && (cSands == map[y][x]);
    };
    with_xy flipxy = [&]WITH_XY {
        map[y][x] = cWater;
    };
    paint8(issand, flipxy)(x, y);
}

void Continent::desertify() {
    visib.visit([&]WITH_XY{
        auto& cell = map[y][x];
        cell = cell == cWater ? cSands : cWoods;
    });
}

unsigned Continent::countSand() const {
    unsigned sands = 0;
    shelf.visit([&]WITH_XY {
        sands += map[y][x] == cSands;
    });
    return sands;
}

void Continent::makeLakes() {
    delugify(kDoAcid);
    desertify();
    irrigate(kMargin, kMargin); // outer sea
    unsigned sands = countSand();
    unsigned lakes = sands * kNLakes / kMapDim;
    for(unsigned fl = 0; fl < lakes; ++fl) {
        Point rp = shelf.rand();
        irrigate(rp.x, rp.y);
    }
    delugify(kDoRain);
}

void Continent::petrify() {
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

    struct Coral { Point p; Shift dir; };
    std::multimap<Real, Coral> candydates; // sic(k)
    // AND-convolve 2x2 wood patches once I have enough spatial imagination
    // MapHolder<char> sqmem{false};
    // ChrMap& sqm = sqmem.map();
    shelf.visit([&]WITH_XY {
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

void Continent::polish() {
    // make sure there are no lone trees (every tree is a part of at least one 2x2 woods square)
    // also applies to sands, mountains, though there may or may not be mountains at this point
    shelf.visit([&]WITH_XY{
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

void Continent::vandalize() {
    unsigned outlier = 0;
    unsigned needcor = 0;

    // not strictly necessary, but reduces the amount of needed correction
    shelf.visit([&]WITH_XY {
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
    // fprintf(stderr, "Hexen spots: %u, Unfixed spots: %u\n", outlier, needcor);
}

bool Continent::is_square(int x, int y, int c) const {
    return map[y+1][x] == c && map[y+1][x+1] == c && map[y][x+1] == c && map[y][x] == c;
}

bool Continent::is_square WITH_XY const {
    char c = map[y][x];
    return map[y+1][x] == c && map[y+1][x+1] == c && map[y][x+1] == c;
}

bool Continent::is_locked WITH_XY const {
    char c = map[y][x];
    return (map[y-1][x] != c || map[y-1][x+1] != c)
        && (map[y+2][x] != c || map[y+2][x+1] != c)
        && (map[y][x-1] != c || map[y+1][x-1] != c)
        && (map[y][x+2] != c || map[y+1][x+2] != c);
}

bool Continent::inlandsq WITH_XY const {
    constexpr char c = cWater;
    return (map[y-1][x] != c && map[y-1][x+1] != c)
        && (map[y+2][x] != c && map[y+2][x+1] != c)
        && (map[y][x-1] != c && map[y+1][x-1] != c)
        && (map[y][x+2] != c && map[y+1][x+2] != c);
}

bool Continent::is_locked_square WITH_XY const {
    return is_square(x, y) && is_locked(x, y);
}

bool Continent::is_hexen WITH_XY const {
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

bool Continent::is_freesq WITH_XY const {
    return !seg[y][x] && !seg[y][x+1] && !seg[y+1][x] && !seg[y+1][x+1];
}

bool Continent::is_friendly(int seg, int ours) {
    return !seg || seg == ours;
}

bool Continent::is_ourssq(int x, int y, int segment) const {
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
}

void Continent::mark_sq(int x, int y, int seg_id) {
    seg[y][x] = seg[y][x+1] = seg[y+1][x] = seg[y+1][x+1] = seg_id;
}

void Continent::thicken(char match, char subst) {
    // first, detect immobile blocks
    auto nobeach = [&]WITH_XY {
        return kSandboat || cSands != subst || inlandsq(x, y);
    };
    shelf.visit([&]WITH_XY {
        if(map[y][x] == match && is_locked_square(x, y) && is_freesq(x, y) && nobeach(x, y)) {
            mark_sq(x, y, alloc_seg());
        }
    });
    shelf.visit([&]WITH_XY {
        if(is_square(x, y, match) && is_freesq(x, y)) {
            const unsigned seg_id = alloc_seg();
            paint4([&]WITH_XY {
                return shelf.covers({x, y}) && is_square(x, y, match) && is_ourssq(x, y, seg_id) && nobeach(x, y);
            }, [&]WITH_XY {
                mark_sq(x, y, seg_id);
            })(x, y);
        }
    });
    unsigned outliers = 0;
    shelf.visit([&]WITH_XY {
        if(map[y][x] == match) {
            if(!seg[y][x]) {
                map[y][x] = cPlain;
                outliers++;
            } else {
                map[y][x] = subst;
            }
        }
    });
    // fprintf(stderr, "Outliers [%c]: %u\n", match, outliers);
}

void Continent::thicken(char match) {
    thicken(match, match); // no substitution
}

void Continent::thicken() {
    thicken(cWoods);
    thicken(cRocks);
    thicken(cSands);
    // thicken(cWater); // needs proper sea shelf bounds
    if(kSuomize) {
        unsigned woods = 0, rocks = 0;
        shelf.visit([&]WITH_XY {
            char& c = map[y][x];
            woods += cWoods == c;
            rocks += cRocks == c;
        });
        bool want_more_woods = cWoods == kSuomize;
        bool have_more_rocks = rocks >= woods;
        if(want_more_woods == have_more_rocks) {
            shelf.visit([&]WITH_XY {
                char& c = map[y][x];
                if(c == cWoods || c == cRocks) {
                    c ^= (cWoods ^ cRocks);
                }
            });
        }
    }
}

void Continent::aridize() {
    if(kAridize) {
        thicken(cPlain, cSands);
    }
}

} // namespace map