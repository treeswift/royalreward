#include "geology.h"
#include "geography.h"
#include "legends.h"
#include "military.h"
#include "mission.h"
#include "savefile.h"

#include <cstdio>
#include <string>
#include <iostream>

char readchar() {
    std::string response;
    std::getline(std::cin, response);
    return *response.c_str() & 0x5f;
}

int main(int argc, char** argv) {
    (void) argc;
    (void) argv;

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

    unsigned seed = 1u; // TODO ask for the seed
                    // TODO ask for the game path

    const char* rank = dat::Prototype::Name(type);
    fprintf(stdout, R"QUOTE(
        %s the %s,

New maps are being charted for you to explore, full of dangers and treasures.
Please wait while I perform _humanlike_ actions to make this game playable --
for it's most humanlike to build upon the work of giants, and ultimately upon
that of our Lord the Creator, Who brought every number, concept and algorithm
into existence before we were born, as will destroy them at the end of times.

)QUOTE", name.c_str(), rank);

    return 0;
}
