#include <cstdio>
#include <cstdlib>
#include <fstream>

#include "savefile.h"

using namespace dat;

int main(int argc, char** argv) {
    Aspects oo;

    // TODO filter out command line switches, modifying args
    // oo.conts = {1, 1};
    //oo.sections &= ~Aspects::Foes;
    //oo.sections &= ~Aspects::Fort;
    //oo.sections &= ~Aspects::Stat;

    // actual output to stdout, human-readable to stderr
    if(argc <= 1) {
        fprintf(stderr, "==== DAT file verifier ====\n");
        fprintf(stderr, "Expected: DAT filename list\n");
        return 1;
    }
    for(int i = 1; i < argc; ++i) {
        std::string filename{argv[i]};
        std::fstream fs(filename, std::ios_base::in);
        if(!fs.good()) {
            fprintf(stderr, "Can't open `%s'\n", filename.c_str());
            continue;
        }
        fs.seekg(0, std::ios_base::end);
        if(fs.tellg() != sizeof(SaveFile)) {
            fprintf(stderr, "`%s' doesn't look like a DAT file\n", filename.c_str());
            continue;
        }
        fs.seekg(0, std::ios_base::beg);
        SaveFile sf;
        fs.read(reinterpret_cast<char*>(&sf), sizeof(SaveFile));
        std::size_t pos = fs.tellg();
        if(pos != sizeof(SaveFile)) {
            fprintf(stderr, "Incomplete read from %s: %lu bytes\n", filename.c_str(), pos);
            continue;
        }
        fprintf(stdout, "\n\n@Path: %s\n", filename.c_str());
        sf.dump(*stdout, oo);
    }

    if(oo.sections & Aspects::Stat) {
        DumpStats(*stdout);
    }
}