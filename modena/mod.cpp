#include "geology.h"
#include "geography.h"
#include "legends.h"
#include "military.h"
#include "mission.h"
#include "savefile.h"
#include "settings.h"

using namespace mod;

int main(int argc, char** argv) {
    (void) argc;
    (void) argv;

    Settings s;
    SetupInteractively(s);

    unsigned seed = 1u; // TODO ask for the seed
                    // TODO ask for the game path

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
