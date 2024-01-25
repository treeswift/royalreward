#include "geology.h"
#include "geography.h"
#include "legends.h"
#include "military.h"
#include "mission.h"
#include "savefile.h"
#include "settings.h"
#include "technology.h"
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
        -l E/N/H/I      difficulty level
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
    geo.kRoamers = 35;
    geo.kDwells = 11; // default
    map::Continent mediocria(geo);
    mediocria.generate();

    geo.kGround = map::kMature;
    geo.kWizard = map::kMature;
    geo.kDPow1 = 0.1f; // no-op
    geo.kDPow2 = 0.2f;
    geo.kCastles = 6;
    geo.kRoamers = 27;
    geo.kDwells = 10;
    map::Continent redwoodia(geo);
    redwoodia.kSuomize = map::cWoods;
    redwoodia.generate();

    geo.kDPow1 = geo.kDPow2 = 0.f;
    geo.kDecay = 0.5;
    geo.kCastles = 6; // no-op
    geo.kRoamers = 10;
    geo.kDwells = 8;
    map::Continent caribbea(geo);
    caribbea.kDoAcid += 2;
    caribbea.generate();

    geo.setComposition(1);
    geo.kCastles = 3;
    geo.kRoamers = 15;
    geo.kDwells = 7;
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
    dat::Leftovers lovers;
    sf.setMission(mission, lovers);

    dat::UIOptions uio;
    uio.sound = 0;
    uio.delay = 1;
    sf.setUIOptions(uio);

    // cheats
    constexpr bool kExploreWorldCheat = false; // dbg
    if(kExploreWorldCheat) {
        memset(sf.cgate, 1, dat::kAlphabet);
        memset(sf.tgate, 1, dat::kAlphabet);
        sf.stasis = 500;
        sf.avail[0] = sf.avail[1] = sf.avail[2] = true;
        sf.visib[0] = sf.visib[1] = sf.visib[2] = sf.visib[3] = true;
        sf.units[0] = mil::Salamanders;
        sf.troops[0] = 20; // just in case
        sf.base_command = sf.curr_command = 5000;
        sf.known[dat::ToFort] = 50;
        sf.known[dat::ToPort] = 50;
        sf.known[dat::Sigint] = 50;
        sf.smart = true;
        sf.storm = true;
    }

    std::string outdir = salad.make(sf, lovers);
    // TODO process success/failure
    constexpr unsigned kQuotes = 7;
    constexpr const char* kGodspeed[7] = {
        " -- and the rest will become history.",
        ", do what you must, and come what may.",
        ", and come back with your shield or on it!",
        " and, whatever happens, never forget to wipe your sword.",
        " and fight as if you are already dead.",
        " and fight not because you hate what is in front of you, but\n because you love what's behind.",
        " -- and either find your way, or make one.",
    };
    unsigned g_pick = (rnd::hwrandom() & 0xffff) % kQuotes;
    fprintf(stdout, "...done. Now step into your time machine, cast \n\n\tcd %s\n"
                    "\n upon arrival%s\n", outdir.c_str(), kGodspeed[g_pick]);

    return 0;
}
