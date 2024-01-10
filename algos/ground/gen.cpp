#include <cstdio>
#include <cstdint>
#include <cstdlib>
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

void genPaint() {
    MapHolder<char> chrmem(kWater);
    ChrMap& chm = chrmem.map();

    std::vector<Paint> features;

    auto drawLine = [&](unsigned x0, unsigned y0, unsigned x1, unsigned y1, char color) {
        features.push_back(Paint{x0, y0, color});

        for(Real f = 0.f; f < 1.f; f += (1.f / kMapDim)) {
            unsigned x = x0 * f + x1 * (1.f - f);
            unsigned y = y0 * f + y1 * (1.f - f);
            if(x != features.back().x || y != features.back().y) {
                features.push_back({x, y, color});
            }
        }
    };

    for(unsigned fl = 0; fl < kNLines; ++fl) {
        unsigned sx = kMinBox + std::rand() % kFeaAmp;
        unsigned sy = kMinBox + std::rand() % kFeaAmp;
        unsigned dx = std::rand() % ( kMapDim - kMinBox - sx );
        unsigned dy = std::rand() % ( kMapDim - kMinBox - sy );

        unsigned x0 = dx + (std::rand() % sx);
        unsigned y0 = dy + (std::rand() % sy);        
        unsigned x1 = dx + (std::rand() % sx);
        unsigned y1 = dy + (std::rand() % sy);

        char color = std::rand() % kMaxCol + 1u;
        drawLine(x0, y0, x1, y1, color);
    }

    // drawLine(0, 0, kMapDim, 0, '\0');
    // drawLine(0, 0, 0, kMapDim, '\0');
    // drawLine(kMapDim, 0, kMapDim, kMapDim, '\0');
    // drawLine(0, kMapDim, kMapDim, kMapDim, '\0');

    for_rect(kWaterz, kWaterz, kMEdgez, kMEdgez, [&]WITH_XY{
        Allegiance all;
        for(const Paint& f : features) {
            int dx = f.x - x;
            int dy = f.y - y;
            Real decay = kDecay;// pow0=archipelago
            decay += f.color * (kDPow2 * f.color + kDPow1);
            // decay *= f.color; 
            // decay *= f.color; // pow2=pangaia/laurasia/gondwana
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
    for_rect(kWaterz, kWaterz, kMEdgez, kMEdgez, [&]WITH_XY{
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
    for(unsigned fl = 0; fl < kNLakes; ++fl) {
        unsigned x = std::rand() % (kMEdgez - kWaterz) + kWaterz;
        unsigned y = std::rand() % (kMEdgez - kWaterz) + kWaterz;
        irrigate(x, y);
    }

    // >4 times inefficient but only done once
    for_rect(kWaterz, kWaterz, kMEdgez, kMEdgez, [&]WITH_XY{
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

    // castle placement
    

    for(unsigned y = 0; y < kMapDim; ++y) {
        std::string line;
        for(unsigned x = 0; x < kMapDim; ++x) {
            line.push_back(map[y][x]);
            line.push_back(map[y][x]);
        }
        fprintf(stdout, "%s\n", line.c_str());
    }
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
