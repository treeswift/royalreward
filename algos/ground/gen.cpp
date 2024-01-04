#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <random>
#include <vector>
#include <algorithm>
#include <functional>

#include "map_defs.h"

/**
 * Generate a map.
 */

void genPaint() {
    std::vector<char> chrvec;
    chrvec.resize(kExtSqr, ' ');
    ChrMap& chm = *reinterpret_cast<ChrMap*>(chrvec.data());

    struct Point {
        std::size_t x, y;
        char color;
    };

    std::vector<Point> features;

    auto drawLine = [&](unsigned x0, unsigned y0, unsigned x1, unsigned y1, char color) {
        features.push_back(Point{x0, y0, color});

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

        std::size_t x0 = dx + (std::rand() % sx);
        std::size_t y0 = dy + (std::rand() % sy);        
        std::size_t x1 = dx + (std::rand() % sx);
        std::size_t y1 = dy + (std::rand() % sy);

        char color = std::rand() % kMaxCol + 1u;
        drawLine(x0, y0, x1, y1, color);
    }

    // drawLine(0, 0, kMapDim, 0, '\0');
    // drawLine(0, 0, 0, kMapDim, '\0');
    // drawLine(kMapDim, 0, kMapDim, kMapDim, '\0');
    // drawLine(0, kMapDim, kMapDim, kMapDim, '\0');

    struct Allegiance {
        float part[kColors];

        Allegiance() { reset(); }

        void reset() { std::fill(part, part + kColors, 0.f); }

    } all;

    for(unsigned y = 0; y < kMapDim; ++y) {
        for(unsigned x = 0; x < kMapDim; ++x) {
            if(x < kWaterz || y < kWaterz || x >= kMEdgez || y >= kMEdgez) {
                continue;
            }
            all.reset();
            for(const Point& f : features) {
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
            std::size_t idx = 0; // guaranteed to be overwritten
            float max = 0.f;
            float sum = 0.f;
            for(std::size_t color = 0; color <= kMaxCol; ++color) {
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
                chm[y][x] = ' ' + idx;
            }
        }
    }

    std::vector<char> chrout{chrvec};
    ChrMap& map = *reinterpret_cast<ChrMap*>(chrout.data());
    for(std::size_t y = kWaterz; y < kMEdgez; ++y) {
        for(std::size_t x = kWaterz; x < kMEdgez; ++x) {
            char color = chm[y][x];
            auto nihilate = [&](std::size_t xo, std::size_t yo) {
                if(color != ' ') {
                    char ocolor = chm[yo][xo];
                    if(ocolor != ' ' && color != ocolor) {
                        map[y][x] = map[yo][xo] = ' ';
                    }
                }
            };
            nihilate(x + 1, y);
            nihilate(x + 1, y + 1);
            nihilate(x,     y + 1);
        }
    }

    // TODO softcode edge
    auto isinner = [&](unsigned x, unsigned y) {
        return x >= kMargin && y >= kMargin && x < kMapMax && y < kMapMax;
    };

    // desertify
    for(unsigned y = 0; y < kMapDim; ++y) {
        for(unsigned x = 0; x < kMapDim; ++x) {
            if(map[y][x] != ' ') {
                map[y][x] = '@';
            } else {
                map[y][x] = '.';
            }
        }
    }
    auto issand = [&](unsigned x, unsigned y) {
        return x < kMapDim && y < kMapDim && ('.' == map[y][x]);
    };
    std::function<void(unsigned, unsigned)> irrigate;
    irrigate = [&](unsigned x, unsigned y) {
        if(issand(x, y)) {
            map[y][x] = ' ';
            irrigate(x - 1u, y);
            irrigate(x + 1u, y);
            irrigate(x, y - 1u);
            irrigate(x, y + 1u);
            irrigate(x - 1u, y - 1u);
            irrigate(x + 1u, y + 1u);
            irrigate(x - 1u, y + 1u);
            irrigate(x + 1u, y - 1u);
        }
    };

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

    for(unsigned y = 0; y < kMapDim; ++y) {
        std::string line;
        for(std::size_t x = 0; x < kMapDim; ++x) {
            line.push_back(map[y][x]);
            line.push_back(map[y][x]);
        }
        fprintf(stdout, "%s\n", line.c_str());
    }
}

int main(int argc, char** argv) {
    // box = 32
    // 1: viable
    // 2: canals
    // 3-7: fpe
    // 8: viable
    std::srand(kSeed);

    // genPlasma();
    genPaint();

    return 0;
}
