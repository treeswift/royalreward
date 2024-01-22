#include "geology.h"
#include "geography.h"
#include "legends.h"
#include "military.h"
#include "mission.h"
#include "savefile.h"
#include "settings.h"
#include "vinegar.h"

using namespace mod;

int help() {
    fprintf(stderr, R"HELP(
    Modena use: 
        mod [-options] <path-to-installation>

    Basic options include:
        -n NAME         player name
        -c A/B/C/D      player class
        -d E/N/H/I      difficulty
        -s NUMBER       random seed

    Run "mod -h" to see a complete list of available tweaks and tricks.

)HELP");
    return 1;
}

int main(int argc, char** argv) {
    Settings s;
    if(argc <= 1) {
        return help();
    }

    s.path = argv[1]; // FIXME replace with a complete option parser
    Salad salad(s.path);
    if(salad.progress < Salad::Marinated) {
        fprintf(stderr, "No installation found at: %s\n", s.path.c_str());
        return -1;
    }
    SetupInteractively(s);
    rnd::seed(s.seed);

    const char* rank = dat::Prototype::Name(s.type);
    fprintf(stdout, R"QUOTE(
        %s the %s,

New maps are being charted for you to explore, full of dangers and treasures.
Please wait while I perform _humanlike_ actions to make this game playable --
for it's most humanlike to build upon the work of giants, and ultimately upon
that of our Lord the Creator, Who brought every number, concept and algorithm
into existence before we were born, as will destroy them at the end of times.

)QUOTE", s.name.c_str(), rank);

    return 0;
}
