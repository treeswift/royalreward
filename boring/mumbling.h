#ifndef _BORING_MUMBLING_H_
#define _BORING_MUMBLING_H_

#include <list>
#include <string>
#include <cstdarg>  // caller convenience
#include <stdexcept> // ditto

// I would REALLY like to use formatted output with e.g. stringstream;
// sadly, https://en.cppreference.com/w/cpp/header/print is C++23 only

namespace mum {

struct scroll {
    int fprintf(const char* fmt, ...);
    int vfprintf(const char* fmt, std::va_list args);

    std::list<std::string> lines;
};

int format(std::string& buf, const char* fmt, ...);
int vformat(std::string& buf, const char* fmt, std::va_list args);

int format(scroll& log, const char* fmt, ...);
int vformat(scroll& log, const char* fmt, std::va_list args);

std::string format(const char* fmt, ...);
std::string vformat(const char* fmt, std::va_list args);

template<typename X>
__attribute__((noreturn)) void bummer(const char* fmt, ...) {
    va_list args; va_start(args, fmt);
    std::string msg = format(fmt, args);
    va_end(args);
    throw X(msg);
}

} // namespace mum

#endif
