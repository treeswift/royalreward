#include "geology.h"
#include "geography.h"
#include "legends.h"
#include "military.h"
#include "mission.h"
#include "savefile.h"
#include "settings.h"
#include "vinegar.h"

#include <cstring> // ugly option parsing

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
    if(argc > 2 && !std::strcmp(argv[2], "-1")) {
        s.name = "Nolan";
        s.type = 3;
        s.level = 3;
        s.seed = 1;
    } else {
        SetupInteractively(s);
    }
    rnd::seed(s.seed);

    dat::Prototype::Type type = (dat::Prototype::Type) s.type; // TODO fix
    dat::Prototype player(type);
    const char* rank = player.Name();
    fprintf(stdout, R"QUOTE(
        %s the %s,

New maps are being charted for you to explore, full of dangers and treasures.
Please wait while I perform _humanlike_ actions to make this game playable --
for it's most humanlike to build upon the work of giants, and ultimately upon
that of our Lord the Creator, Who brought every number, concept and algorithm
into existence before we were born, as will destroy them at the end of times.

)QUOTE", s.name.c_str(), rank);

    map::Geology geo;
    geo.setComposition(7);
    geo.kGround = geo.kIsland;
    geo.kWizard = player.is_a_bookworm ? map::kMature : geo.kIsland;
    geo.kDecay = 0.f;
    geo.kDPow1 = 0.1f;
    geo.kDPow2 = 0.f;
    geo.kCastles = 11;
    map::Continent mediocria(geo);
    mediocria.generate();

    geo.kGround = map::kMature;
    geo.kWizard = map::kMature;
    geo.kDPow1 = 0.1f; // no-op
    geo.kDPow2 = 0.2f;
    geo.kCastles = 6;
    map::Continent redwoodia(geo);
    redwoodia.kSuomize = map::cWoods;
    redwoodia.generate();

    geo.kDPow1 = geo.kDPow2 = 0.f;
    geo.kDecay = 0.5;
    geo.kCastles = 6; // no-op
    map::Continent caribbea(geo);
    caribbea.kDoAcid += 2;
    caribbea.generate();

    geo.setComposition(1);
    geo.kCastles = 3;
    map::Continent desertia(geo);
    desertia.kAridize = true;
    desertia.kSuomize = map::cRocks;
    desertia.generate();

    dat::Mission mission;
    mission.chart(mediocria, 6);
    mission.chart(redwoodia, 4);
    mission.chart(caribbea, 4);
    mission.chart(desertia, 3);

    dat::SaveFile sf;
    sf.setHeroName(s.name);
    sf.setHeroType(type);
    sf.setHeroLoc({11, 5});
    sf.setLevel(s.level);
    sf.setUIOptions();
    dat::Leftovers lovers;
    sf.setMission(mission, lovers);

    // TODO write sf
    // TODO patch rr

    return 0;
}
