#include <cstdio>
#include <cstdlib>
#include <fstream>

#include "savefile.h"

using namespace dat;

// TODO filter out command line switches, modifying args

int main(int argc, char** argv) {
    // actual output to stdout, human-readable to stderr
    fprintf(stderr, "\n==== DAT file verifier ====\n\n");
    if(argc <= 1) {
        fprintf(stderr, "Expected: DAT filename list\n");
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
        fprintf(stdout, "@Path: %s\n", filename.c_str());
        *stdout << sf;
    }
}