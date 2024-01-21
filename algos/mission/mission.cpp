#include "mission.h"

#include "dat_defs.h"

#include <cstdlib>
#include <cstdio>

namespace dat {

namespace {

constexpr const char kCastles[kContinents] = {11, 6, 6, 3}; // total 26
constexpr const char kEnemies[kContinents] =  {6, 4, 4, 3}; // total 17

constexpr unsigned kRanks = 4;

const char* kCastes[][kRanks] = {
    {
        "Cadet",
        "Lieutenant",
        "Major",
        "Colonel"
    },
    {
        "Volunteer",
        "Activist",
        "Ambassador",
        "Champion"
    },
    {
        "Student",
        "Intern",
        "Master",
        "Doctor"
    },
    {
        "Athlete",
        "Participant",
        "Contender",
        "Medalist"
    },
};

} // anonymous

const char* Prototype::Name(int t, int rk) { return kCastes[t][rk]; }

Prototype::Prototype(Type t) {
    using namespace mil;
    switch (type = t)
    {
    case A:
        command = 100;
        army.push_back({20, Recruits});
        army.push_back({2, Bowmen});
        salary  = 1000;
        savings = 7500;
        is_a_bookworm = false;
        intuition = 1;
        education = 2;
        break;

    case B:
        command = 80;
        army.push_back({20, Housewives});
        army.push_back({20, Recruits});
        salary  = 1000;
        savings = 10000;
        is_a_bookworm = false;
        intuition = 1;
        education = 3;
        break;

    case C:
        command = 60;
        army.push_back({30, Housewives});
        army.push_back({10, Fairies});
        salary  = 3000;
        savings = 10000;
        is_a_bookworm = true;
        intuition = 2;
        education = 5;
        break;

    case D:
        command = 100;
        army.push_back({20, Canines});
        salary  = 2000;
        savings = 7500;
        is_a_bookworm = false;
        intuition = 0;
        education = 2;
        break;

    default:
        fprintf(stderr, "No such character class: %d\n", t);
        abort();
    }
}

} // namespace dat
