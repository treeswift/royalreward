#include "mumbling.h"

#include <cstdio>

namespace mum {

// when I told you it would be boring, I meant that

int format(scroll& log, const char* fmt, ...) {
    std::va_list args;
    va_start(args, fmt);
    int rv = vformat(log, fmt, args);
    va_end(args);
    return rv;
}

int scroll::fprintf(const char* fmt, ...) {
    std::va_list args;
    va_start(args, fmt);
    int rv = vfprintf(fmt, args);
    va_end(args);
    return rv;
}

int vformat(scroll& log, const char* fmt, std::va_list args) {
    return log.vfprintf(fmt, args);
}

int scroll::vfprintf(const char* fmt, std::va_list args) {
    lines.push_back({});
    std::size_t reserve = std::vsnprintf(nullptr, 0, fmt, args);
    std::string& back = lines.back();
    back.resize(reserve); // C++11 guarantees that \0 is stored
    return std::vsnprintf(&back[0], reserve, fmt, args);
}

} // namespace mum
