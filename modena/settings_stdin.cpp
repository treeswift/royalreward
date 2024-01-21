#include "settings.h"
#include "savefile.h"
#include "mission.h"

#include <cstdio>
#include <string>
#include <iostream>

namespace mod {

char readchar() {
    std::string response;
    std::getline(std::cin, response);
    return *response.c_str() & 0x5f;
}

void SetupInteractively(Settings& s) {
    auto yesorno = []() {
        while(true) {
            fprintf(stdout, "[Y]es or [n]o: ");
            char c = readchar();
            if(c == 'N') return false;
            if(c == 'Y') return true;
        }
    };

    // enough of vinegar for you for now
    fprintf(stdout, R"INTRO(
        Welcome to Modena!

Modena is a city in Italy, known for its university (founded in 1175 -- in the
era of knights and crusades), its car industry and its world-renowned blend of
balsamic vinegar.
One can do a _lot_ of things with vinegar. It reveals and highlights the taste
of dishes, and it's also good for preserving your favorites for decades. How's
that related to computers? If the words "Royal Reward" or "Caliph's Honor", or
possibly even "Lenin's Prize" ring a certain bell for you, then you are at the
right place.

I am going to ask you a few questions first. If you'd like to speed things up,
so that I cooked "your usual" without any hesitation, press Ctrl+C and type

    mod -h

on the command line. I am a friend of certain scrolls... scribes... wait. They
are called "scripts". Alas, I am getting older myself, but given the likeliest
reason for your visit, may it be accurate to say that it wouldn't be an issue?

Anyway. First, if you don't mind, introduce youself.
You can use a totally made-up name, just keep in under 11 characters.

Who are you? )INTRO");

    auto requery = [&yesorno](const std::string& name) {
        fprintf(stdout, "I have compacted the name you entered to: %s.\n"
            "Is that okay? ", name.c_str());
        return !yesorno();
    };
    auto repeat = [](const std::string& ask_again) {
        std::cout << ask_again;
        return true;
    };

    std::string raw_name;
    std::string name;
    do {
        std::getline(std::cin, raw_name);
        name = raw_name;
        dat::SaveFile::sanitize(name);
    } while(raw_name != name && requery(name) && repeat("Then who are you? "));

    fprintf(stdout, R"ASPIRE(
Very well. Now tell me something about your aspirations.
Which of the following occupations do you most easily associate yourself with?

)ASPIRE");

    constexpr char base = 'A';
    int up_to = dat::Prototype::Total - 1;
    for(char t = 0; t <= up_to; ++t) {
        fprintf(stdout, "%c. %s\n", base + t, dat::Prototype::Name(t));
    }
    char type = -1;
    while(type < dat::Prototype::A || type > up_to) {
        fprintf(stdout, "\n%c-%c: ", base, base + up_to);
        type = readchar() - base;
    }

    fprintf(stdout, R"LEVEL(
Impressive! Now tell me about your expertise in the area of treasure recovery.
Assuming a certain Golden Key, critically important for the well-being of the
human race and the future of civilization, is buried into the soil of exactly
one of four roughly equivalent by their bounding rectangle, but in most other
ways totally different landmasses and/or continental plates, what's your best-
effort recovery ETA? Choose the deadline that reflects your estimate best:

"[E]asy"         (in under 900 days);
"[N]ormal"       (in under 600 days);
"[H]ard"         (in under 400 days);
"[I]mpossible" (in 200 days or less)?

)LEVEL");

    std::string enhi{"ENHI"};
    char eta = -1;
    std::size_t level;
    while((level = enhi.find(eta)) == std::string::npos) {
        fprintf(stdout, "E/N/H/I: ");
        eta = readchar();
    }

    s.seed = rnd::hwrandom();

    fprintf(stdout, R"SEED(
We are getting closer. Now, the very goal of preservation (food or digital) is
to avoid surprises. We have randomly obtained a certain Seed from which all the
upcoming choices are about to be made. You can proceed with it (putting it down
if you like) or enter a predictable and familiar one, such as 1, 2, or 125. You
even enter 0. We are in the Woods-between-the-worlds now. Here is our sugestion
--  keep it (press Enter) or enter a new one?

[%u] )SEED", s.seed);

    std::string seed;
    std::getline(std::cin, seed);
    if(seed.size()) {
        s.seed = std::stoul(seed);
    }

    fprintf(stdout, "Seed chosen: %u\n\n", s.seed);

    // inline the following "out" assignments
    s.name = name;
    s.type = type;
    s.level = level;
    s.seed = 1u;
}

} // namespace mod