#include "map_defs.h"

#include <algorithm>
#include <random>
#include <vector>

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

    std::vector<Real> elevec{0.f, kExtSqr};
    EleMap & elemat = *reinterpret_cast<EleMap *>(elevec.data());
    // EleMap elemat;
    // std::fill(elemat, 0, sizeof(elemat));

    std::mt19937 engine;
    std::uniform_real_distribution<> distro{0., 1.};
    for(int i = 0; i < 4; ++i) distro(engine); // idle runs

    Real scale = 1.f;
    Real downscale = 0.5f;
    constexpr Real kSeas = 0.5;
    constexpr Real kPass = 0.7;
    constexpr Real kWood = 0.9;
    Real concavity = 0.f;//.25f;
    Real elemax = 0.f, elemin = 0.f;
    for(unsigned step = kMapDim; step > 1; ) {
        unsigned next = step >> 1;
        // first pass:
        // v * v
        // - - - 
        // v * v
        for(unsigned y = 0; y <= kMapDim; y += step) {
            for(unsigned x = next; x <= kMapDim; x += step) {
                Real adjust = (distro(engine) - concavity) * scale;
                Real ele = (elemat[y][x-next] + elemat[y][x+next]) * 0.5 + adjust;
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
        for(unsigned x = 0; x <= kMapDim; x += next) {
            for(unsigned y = next; y <= kMapDim; y += step) {
                Real adjust = (distro(engine) - concavity) * scale;
                Real ele = (elemat[y-next][x] + elemat[y+next][x]) * 0.5 + adjust;
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

    Real correction = 1.f / (elemax - elemin);
    for(unsigned y = 0; y < kMapDim; ++y) {
        std::string line;
        for(unsigned x = 0; x < kMapDim; ++x) {
            // char c = ' ' + std::min(std::max(elemat[y][x], 0.f), 1.f) * (0x7f - ' ');
            // char c = (elemat[y][x] - elemin) * correction + lower;
            Real nel = (elemat[y][x] - elemin) * correction;
            line.push_back(nel < kPass  ? nel < kSeas ? ' ' :
                                                        '+'
                                        : nel < kWood ? 'T' :
                                                        '^');
        }
        fprintf(stdout, "%s\n", line.c_str());
    }
}
