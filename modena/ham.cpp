#include "lute_tune.h"
#include "savefile.h" // lovers
#include "hamming.h"
#include "vinegar.h"

#include <fstream>

std::map<std::ptrdiff_t, std::string> legend;

int main(int argc, char** argv) {
    if(argc < 2) {
        fprintf(stderr, "Usage: ham <original-binary>\n");
        exit(-1);
    }

    const char* fname = argv[1];
    std::string output = mod::unpack(fname);

    constexpr auto kRandomAccess = std::ios_base::in | std::ios_base::out | std::ios_base::binary;
    std::fstream hog(output.c_str(), kRandomAccess); // TODO support stdin

    mod::Leftovers lovers;
    mod::old_tune(lovers);

    mod::Slicer slicer;
    lovers.analyze(hog, slicer, legend);

    for(const auto& result : slicer.found_log) {
        std::string label = legend.at(result.first); // token -> label
        label = label.substr(label.find(".") + 1); // remove "struct name."
        fprintf(stdout, "%s: 0x%lx # (hamming=%u confidence=%.2f%%)\n",
            label.c_str(), result.second.pos,
            result.second.difference,
            result.second.confidence * 100.f);
    }
}
