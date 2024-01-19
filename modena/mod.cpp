#include <cstdio>

int main(int argc, char** argv) {
    (void) argc;
    (void) argv;
    // enough of vinegar for you for now
    
    const char* name = "Faust";
    const char* rank = "Student";

    fprintf(stdout, R"QUOTE(
        %s the %s,

A new game is being created; this time indeed new.
Please wait while I perform _humanlike_ actions to make this game playable --
for it is most humanlike to build upon the work of giants, and ultimately upon
that of our Lord the Creator, Who alone created numbers and all other concepts
of mathematics and will destroy them at the end of times.

)QUOTE", name, rank);

    return 0;
}
