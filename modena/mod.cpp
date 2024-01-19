#include <cstdio>

int main(int argc, char** argv) {
    (void) argc;
    (void) argv;
    // enough of vinegar for you for now
    
    const char* name = "Faust";
    const char* rank = "Student";

    fprintf(stdout, R"QUOTE(
        %s the %s,

New maps are being charted for you to explore, full of dangers and treasures.
Please wait while I perform _humanlike_ actions to make this game playable --
for it's most humanlike to build upon the work of giants, and ultimately upon
that of our Lord the Creator, Who brought every number, concept and algorithm
into existence before we were born, as will destroy them at the end of times.

)QUOTE", name, rank);

    return 0;
}
