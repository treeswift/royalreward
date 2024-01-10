#include <cstdio>
#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
#include <functional>

#include "maps.h"

namespace map{

/**
 * Generate a map.
 */

struct Paint {
    unsigned x, y;
    char color;
};

struct Allegiance {
    float part[kColors];

    Allegiance() { reset(); }

    void reset() { std::fill(part, part + kColors, 0.f); }
};

bool goodPointForCastle(char c) {
    return kWoods == c || kRocks == c || kSands == c;
}

bool goodPointForCEntry(char c) {
    return goodPointForCastle(c) || kPlain == c;
}

class Continent {
public:
private:
};

void genPaint() {
    MapHolder<char> chrmem(kWater);
    ChrMap& chm = chrmem.map();

    std::vector<Paint> features;

    auto stroke = [&](unsigned x0, unsigned y0, unsigned x1, unsigned y1, char color) {
        features.push_back(Paint{x0, y0, color});

        for(Real f = 0.f; f < 1.f; f += (1.f / kMapDim)) {
            unsigned x = x0 * f + x1 * (1.f - f);
            unsigned y = y0 * f + y1 * (1.f - f);
            if(x != features.back().x || y != features.back().y) {
                features.push_back({x, y, color});
            }
        }
    };

    Block sbox = bound(kMinBox, kMinBox + kFeaAmp);
    Point base = corner(0);
    Shift dmax = diag(kMapDim - kMinBox);

    for(unsigned fl = 0; fl < kNLines; ++fl) {
        Shift s = sbox.rand() - base;
        Point d = base + (dmax - s).rand();

        Point p0 = d + s.rand();
        Point p1 = d + s.rand();

        char color = rnd::upto(kMaxCol) + 1u;
        stroke(p0.x, p0.y, p1.x, p1.y, color);
    }

    Block visib = bound(0, kMapDim);
    Block conti = visib.inset(kShoalz);
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

        if(max * kWinner > sum && (lns + edge_d > kThorne)) { // majority winner
            chm[y][x] = kWater + idx;
        }
    });

    MapHolder<char> chrout{chrmem};
    ChrMap& map = chrout.map();
    conti.visit([&]WITH_XY{
        char color = chm[y][x];
        auto segregate = [&](unsigned xo, unsigned yo) {
            if(color != kWater) {
                char ocolor = chm[yo][xo];
                if(ocolor != kWater && color != ocolor) {
                    map[y][x] = map[yo][xo] = kWater;
                }
            }
        };
        segregate(x + 1, y);
        segregate(x + 1, y + 1);
        segregate(x,     y + 1);
    });

    // desertify
    for_rect(0, 0, kMapDim, kMapDim, [&]WITH_XY{
        auto& cell = map[y][x];
        cell = cell == kWater ? kSands : kWoods;
    });
    bool_xy issand = [&]WITH_XY {
        return x < kMapDim && y < kMapDim && (kSands == map[y][x]);
    };
    with_xy flipxy = [&]WITH_XY {
        map[y][x] = kWater;
    };
    with_xy irrigate = paint8(issand, flipxy);

    // FIXME:
    // - iterate top down;
    // - irrigate a lake;
    // - count lakes;
    // - make sure (in random order) that every lake has a city on its outermost shore (except the sea where any shore counts)
    // - if city placement fails, keep the lake a desert
    irrigate(kMargin, kMargin);
    Block continent = bound(kShoalz, kMEdgez);
    for(unsigned fl = 0; fl < kNLakes; ++fl) {
        Point rp = continent.rand();
        irrigate(rp.x, rp.y);
    }

    // castle placement
    constexpr unsigned kCastles = 9;
    unsigned castles = 0;
    unsigned y = 0;
    unsigned x;
    auto goodPlaceForCastle = [&]WITH_XY {
        return goodPointForCastle(map[y][x]) && goodPointForCastle(map[y][x+1]) && goodPointForCastle(map[y][x-1])
        && goodPointForCastle(map[y+1][x]) && goodPointForCastle(map[y+1][x+1]) && goodPointForCastle(map[y+1][x-1])
        && goodPointForCEntry(map[y-1][x]);
    };
    while(castles < kCastles) {
        // y = (--y) % kMapDim;
        Point gate = continent.rand();
        x = gate.x, y = gate.y;
        if(goodPointForCastle(map[y][x]) && goodPointForCastle(map[y+1][x])) {
            // tight placement adjustment
            if(!(goodPointForCastle(map[y][x-1]) && goodPointForCastle(map[y+1][x-1]))) {
                ++x;
            } else if(!(goodPointForCastle(map[y][x+1]) && goodPointForCastle(map[y+1][x+1]))) {
                --x;
            }
            if(goodPlaceForCastle(x, y)) {
                int resistance = 0;
                // possibly promote the castle north(view coordinates) [=south(map coordinates)]
                while(rnd::upto(100) > resistance && goodPointForCastle(map[y+2][x-1]) &&
                    goodPointForCastle(map[y+2][x]) && goodPointForCastle(map[y+2][x+1])) {
                        ++y;
                        resistance += 10;
                    }

                // FIXME make transactional!
                map[y][x-1] = map[y+1][x-1] = '[';
                map[y][x] = 'F';
                map[y+1][x] = '1' + castles;
                map[y][x+1] = map[y+1][x+1] = ']';
                map[y-1][x] = kPlain; // etc.etc.etc.
                ++castles; // if tran commit
            }
        }
    }

    // make sure there are no lone trees (every tree is a part of at least one 2x2 woods square)
    // also applies to mountains, though there may or may not be mountains at this point
    conti.visit([&]WITH_XY{
        char color = map[y][x];
        auto isalso = [&](int dx, int dy) {
            return map[y+dy][x+dx] == color
                   && map[y+dy][x] == color
                   && map[y][x+dx] == color;
        };
        if(kWoods == color || kRocks == color) {
            isalso(-1,-1) || isalso(-1,1) || isalso(1,-1) || isalso(1,1) || (map[y][x] = kPlain);
        }
    });

    *stdout << map;
}

} // namespace map

int main(int argc, char** argv) {
    // box = 32
    // 1: viable
    // 2: canals
    // 3-7: fpe
    // 8: viable
    std::srand(map::kSeed);

    map::genPaint();

    return 0;
}
