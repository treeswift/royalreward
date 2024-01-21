#ifndef _MODENA_SETTINGS_H_
#define _MODENA_SETTINGS_H_

#include <string>

namespace mod
{

struct Settings {
    std::string name;
    unsigned type;
    unsigned level;
    unsigned seed;
    std::string path;
};

void SetupInteractively(Settings& s);

}

#endif
