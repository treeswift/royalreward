#ifndef _BORING_MUMBLING_H_
#define _BORING_MUMBLING_H_

#include <cstdarg>
#include <list>
#include <string>

// I would REALLY like to use formatted output with e.g. stringstream;
// sadly, https://en.cppreference.com/w/cpp/header/print is C++23 only

namespace hlp {

struct scroll {
    int fprintf(const char* fmt, ...);
    int vfprintf(const char* fmt, std::va_list va_list);

    std::list<std::string> lines;
};

int format(scroll& log, const char* fmt, ...);
int vformat(scroll& log, const char* fmt, std::va_list va_list);

} // namespace hlp

#endif
