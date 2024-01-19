#include "mission.h"

#include <cstdlib>
#include <cstdio>

namespace dat {

namespace {

const char* kCastes[] = {
    "Knight",
    "Paladin",
    "Sorceress",
    "Barbarian",
};

}

const char* Prototype::Name(Type t) { return kCastes[t]; }

Prototype::Prototype(Type t) {
    using namespace mil;
    switch (type = t)
    {
    case A:
        command = 100;
        army.push_back({Unit::Recruits, 20});
        army.push_back({Unit::Bowmen, 2});
        salary  = 1000;
        savings = 7500;
        is_a_bookworm = false;
        intuition = 1;
        education = 2;
        break;

    case B:
        command = 80;
        army.push_back({Unit::Housewives, 20});
        army.push_back({Unit::Recruits, 20});
        salary  = 1000;
        savings = 10000;
        is_a_bookworm = false;
        intuition = 1;
        education = 3;
        break;

    case C:
        command = 60;
        army.push_back({Unit::Housewives, 30});
        army.push_back({Unit::Fairies, 10});
        salary  = 3000;
        savings = 10000;
        is_a_bookworm = true;
        intuition = 2;
        education = 5;
        break;

    case D:
        command = 100;
        army.push_back({Unit::Canines, 20});
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
