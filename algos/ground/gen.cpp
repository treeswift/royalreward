#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <random>
#include <vector>
#include <algorithm>

constexpr std::size_t kMapDim = 64;
constexpr std::size_t kMapMem = kMapDim * kMapDim;

constexpr std::size_t kMapExt = kMapDim + 1u;
constexpr std::size_t kExtSqr = kMapExt * kMapExt;
using ChrMap = char [kMapExt][kMapExt];
using EleMap = float[kMapExt][kMapExt];

/**
 * Generate a map.
 */
void genPlasma() {
    // DoD: a 64x64 map output file + text output
    // Legend:
    // ^ mountains
    // T trees
    //   space (plains)
    // ~ sea
    // # castle
    // H castle gate
    // / or \ town
    // $ treasure
    // * warrior
    // + volunteer
    // > or < sign
    // o or O cave

    std::vector<float> elevec{0.f, kExtSqr};
    EleMap & elemat = *reinterpret_cast<EleMap *>(elevec.data());
    // EleMap elemat;
    // std::fill(elemat, 0, sizeof(elemat));

    std::mt19937 engine;
    std::uniform_real_distribution<> distro{0., 1.};
    for(int i = 0; i < 4; ++i) distro(engine); // idle runs

    float scale = 1.f;
    float downscale = 0.5f;
    constexpr float kSeas = 0.5;
    constexpr float kPass = 0.7;
    constexpr float kWood = 0.9;
    float concavity = 0.f;//.25f;
    float elemax = 0.f, elemin = 0.f;
    for(std::size_t step = kMapDim; step > 1; ) {
        std::size_t next = step >> 1;
        // first pass:
        // v * v
        // - - - 
        // v * v
        for(std::size_t y = 0; y <= kMapDim; y += step) {
            for(std::size_t x = next; x <= kMapDim; x += step) {
                float adjust = (distro(engine) - concavity) * scale;
                float ele = (elemat[y][x-next] + elemat[y][x+next]) * 0.5 + adjust;
                elemax = std::max(elemax, ele);
                elemin = std::min(elemin, ele);
                // fprintf(stdout, "<%lu, %lu> += %f = %f\n", x, y, adjust, ele);
                elemat[y][x] = ele;
            }
        }
        // second pass:
        // v v v
        // * * *
        // v v v
        for(std::size_t x = 0; x <= kMapDim; x += next) {
            for(std::size_t y = next; y <= kMapDim; y += step) {
                float adjust = (distro(engine) - concavity) * scale;
                float ele = (elemat[y-next][x] + elemat[y+next][x]) * 0.5 + adjust;
                elemax = std::max(elemax, ele);
                elemin = std::min(elemin, ele);
                // fprintf(stdout, "<%lu, %lu> += %f = %f\n", x, y, adjust, ele);
                elemat[y][x] = ele;
            }
        }
        step = next;
        scale *= downscale;
    }

    fprintf(stdout, "Range: %f .. %f\n", elemin, elemax);
    fprintf(stdout, "<0, 0> = %f\n", elemat[0][0]);

    float correction = 1.f / (elemax - elemin);
    for(std::size_t y = 0; y < kMapDim; ++y) {
        std::string line;
        for(std::size_t x = 0; x < kMapDim; ++x) {
            // char c = ' ' + std::min(std::max(elemat[y][x], 0.f), 1.f) * (0x7f - ' ');
            // char c = (elemat[y][x] - elemin) * correction + lower;
            float nel = (elemat[y][x] - elemin) * correction;
            line.push_back(nel < kPass  ? nel < kSeas ? ' ' :
                                                        '+'
                                        : nel < kWood ? 'T' :
                                                        '^');
        }
        fprintf(stdout, "%s\n", line.c_str());
    }
}

void genPaint() {
    std::vector<char> chrvec;
    chrvec.resize(kExtSqr, ' ');
    ChrMap& chm = *reinterpret_cast<ChrMap*>(chrvec.data());

    constexpr std::size_t kWaterz = 3u;
    constexpr std::size_t kMEdgez = kMapDim - kWaterz;
    constexpr std::size_t kMinBox = 2u;
    constexpr std::size_t kFeaAmp = 24u;
    constexpr char kColors = 15;
    constexpr char kMaxCol = 7;
    struct Point {
        std::size_t x, y;
        char color;
    };

    std::vector<Point> features;

    constexpr std::size_t flines = 16;

    auto drawLine = [&](std::size_t x0, std::size_t y0, std::size_t x1, std::size_t y1, char color) {
        features.push_back(Point{x0, y0, color});

        for(float f = 0.f; f < 1.f; f += (0.5f / kMapDim)) {
            std::size_t x = x0 * f + x1 * (1.f - f);
            std::size_t y = y0 * f + y1 * (1.f - f);
            if(x1 != features.back().x || y1 != features.back().y) {
                features.push_back({x1, y1, color});
            }
        }
    };

    for(std::size_t fl = 0; fl < flines; ++fl) {
        std::size_t sx = kMinBox + std::rand() % kFeaAmp;
        std::size_t sy = kMinBox + std::rand() % kFeaAmp;
        std::size_t dx = std::rand() % ( kMapDim - sx );
        std::size_t dy = std::rand() % ( kMapDim - sy );

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

    constexpr float kDecay = 0.04f;

    for(std::size_t y = 0; y < kMapDim; ++y) {
        for(std::size_t x = 0; x < kMapDim; ++x) {
            if(x < kWaterz || y < kWaterz || x >= kMEdgez || y >= kMEdgez) {
                continue;
            }
            all.reset();
            for(const Point& f : features) {
                int dx = f.x - x;
                int dy = f.y - y;
                all.part[f.color] += expf(- kDecay * (dx * dx + dy * dy));
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
            if(max * 2.f > sum) { // majority winner
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

    for(std::size_t y = 0; y < kMapDim; ++y) {
        std::string line;
        for(std::size_t x = 0; x < kMapDim; ++x) {
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
    std::srand(1u);

    // genPlasma();
    genPaint();

    return 0;
}
