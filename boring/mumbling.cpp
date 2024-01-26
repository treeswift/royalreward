#include "mumbling.h"

#include <cstdio>

namespace mum {

// when I told you it would be boring, I meant that

#define VA_FORWARD(fname, ...) \
    { \
        std::va_list args; \
        va_start(args, fmt); \
        int rv = v##fname(__VA_ARGS__, args); \
        va_end(args); \
        return rv; \
    }

int format(scroll& log, const char* fmt, ...) {
    VA_FORWARD(format, log, fmt);
}

int vformat(scroll& log, const char* fmt, std::va_list args) {
    return log.vfprintf(fmt, args);
}

int format(std::string& buf, const char* fmt, ...) {
    VA_FORWARD(format, buf, fmt);
}

int vformat(std::string& buf, const char* fmt, std::va_list args) {
    std::size_t reserve = std::vsnprintf(nullptr, 0, fmt, args);
    buf.resize(reserve); // C++11 guarantees that \0 is stored
    return std::vsnprintf(&buf[0], reserve, fmt, args);
}

int scroll::fprintf(const char* fmt, ...) {
    VA_FORWARD(fprintf, fmt);
}

int scroll::vfprintf(const char* fmt, std::va_list args) {
    lines.push_back({});
    return vformat(lines.back(), fmt, args);
}

std::string format(const char* fmt, ...) {
    std::va_list args;
    va_start(args, fmt);
    std::string buf;
    vformat(buf, fmt, args);
    va_end(args);
    return buf;
}

std::string vformat(const char* fmt, std::va_list args) {
    std::string buf;
    return vformat(buf, fmt, args), buf;
}

} // namespace mum
