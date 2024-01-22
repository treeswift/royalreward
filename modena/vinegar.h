#ifndef _MODENA_VINEGAR_H_
#define _MODENA_VINEGAR_H_

#include "savefile.h"

#include <string>
#include <map>

namespace mod {

struct Salad {
enum State {
    Initial=0,
    Dir_Found,
    Has_Cache,
    Marinated,

} progress = Initial;

std::string path;
std::map<std::string, std::string> nutrients;

Salad(const std::string& dir);
void list_nutrient(const std::string& fname);
std::string make(const dat::SaveFile& sf, const dat::Leftovers& lovers);
};

} // namespace mod

#endif
