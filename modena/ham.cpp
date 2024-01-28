#include "lute_tune.h"
#include "savefile.h" // lovers
#include "hamming.h"
#include "vinegar.h"

#include <fstream>

mod::Leftovers lovers;
mod::Slicer slicer;

std::map<std::ptrdiff_t, std::string> legend;

template<std::size_t kSz>
void search(const char (&ref)[kSz], const char* label) {
    std::ptrdiff_t fieldoff = reinterpret_cast<const char*>(&ref)
                            - reinterpret_cast<const char*>(&lovers);
    mod::Hamming::Cfg cfg((mod::Token) fieldoff, std::string{ref, ref + kSz});
    slicer.search(cfg);
    legend[fieldoff] = label;
}

#define SEARCH(field) \
    { \
        search(field, #field); \
    }

int main(int argc, char** argv) {
    if(argc < 2) {
        fprintf(stderr, "Usage: ham <original-binary>\n");
        exit(-1);
    }

    const char* fname = argv[1];
    std::string output = mod::unpack(fname);

    mod::old_tune(lovers);
    constexpr unsigned X = mod::Leftovers::X;
    constexpr unsigned Y = mod::Leftovers::Y;

    SEARCH(lovers.conts);
    SEARCH(lovers.forts[X]);
    SEARCH(lovers.forts[Y]);
    SEARCH(lovers.ports[X]);
    SEARCH(lovers.ports[Y]);
    SEARCH(lovers.p_bay[X]);
    SEARCH(lovers.p_bay[Y]);
    SEARCH(lovers.p_air[X]);
    SEARCH(lovers.p_air[Y]);
    SEARCH(lovers.ptofs);

    constexpr auto kRandomAccess = std::ios_base::in | std::ios_base::out | std::ios_base::binary;
    std::fstream hog(output.c_str(), kRandomAccess); // TODO support stdin
    while(!hog.eof()) {
        auto p = hog.tellg();
        char c = hog.get();
        slicer.suggest(p, c);
    }

    for(const auto& result : slicer.found_log) {
        std::string label = legend.at(result.first); // token -> label
        label = label.substr(label.find(".") + 1); // remove "struct name."
        fprintf(stdout, "%s: 0x%lx # (hamming=%u confidence=%.2f%%)\n",
            label.c_str(), result.second.pos,
            result.second.difference,
            result.second.confidence * 100.f);
    }
}
