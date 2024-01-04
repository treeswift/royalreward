#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <random>
#include <vector>

constexpr std::size_t kMapDim = 64;
constexpr std::size_t kMapMem = kMapDim * kMapDim;

/**
 * Generate a map.
 */
int main(int argc, char** argv) {
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

    constexpr std::size_t kMapExt = kMapDim + 1u;
    constexpr std::size_t kExtSqr = kMapExt * kMapExt;
    std::vector<float> elevec{0.f, kExtSqr};
    using EleMap = float[kMapExt][kMapExt];
    EleMap & elemat = *reinterpret_cast<EleMap *>(elevec.data());
    // EleMap elemat;
    // std::fill(elemat, 0, sizeof(elemat));

    std::mt19937 engine;
    std::uniform_real_distribution<> distro{0., 1.};

    // float scale = 1.f / kMapDim / kMapDim;
    float concavity = 0.f;//.25f;
    float elemax = 0.f, elemin = 0.f;
    for(std::size_t step = kMapDim; step > 1; ) {
        std::size_t next = step >> 1;
        float spread = next;
        // first pass:
        // v * v
        // - - - 
        // v * v
        for(std::size_t y = 0; y <= kMapDim; y += step) {
            for(std::size_t x = next; x <= kMapDim; x += step) {
                float adjust = (distro(engine) - concavity) * spread;
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
                float adjust = (distro(engine) - concavity) * spread;
                float ele = (elemat[y-next][x] + elemat[y+next][x]) * 0.5 + adjust;
                elemax = std::max(elemax, ele);
                elemin = std::min(elemin, ele);
                // fprintf(stdout, "<%lu, %lu> += %f = %f\n", x, y, adjust, ele);
                elemat[y][x] = ele;
            }
        }
        step = next;
    }

    fprintf(stdout, "Range: %f .. %f\n", elemin, elemax);
    fprintf(stdout, "<0, 0> = %f\n", elemat[0][0]);

    unsigned upper = 0x7f;
    unsigned lower = 0x00;
    float correction = (upper - lower) / (elemax - elemin);
    for(std::size_t y = 0; y < kMapDim; ++y) {
        std::string line;
        for(std::size_t x = 0; x < kMapDim; ++x) {
            // char c = ' ' + std::min(std::max(elemat[y][x], 0.f), 1.f) * (0x7f - ' ');
            char c = (elemat[y][x] - elemin) * correction + lower;
            line.push_back(c < '`' ? c < '0' ? '~' : ' ' : c < 'p' ? 'T' : '^');
        }
        fprintf(stdout, "%s\n", line.c_str());
    }

    return 0;
}
