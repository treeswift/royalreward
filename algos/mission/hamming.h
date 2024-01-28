#ifndef _MODENA_HAMMING_H_
#define _MODENA_HAMMING_H_

#include <functional>
#include <istream>
#include <string>
#include <list>
#include <map>

namespace mod {

using Token = std::uintptr_t;

enum class Origin {
    STR,
    LUT,
};

struct Patch {
    Origin origin;
    Token selector;
    std::size_t size;

    bool operator==(const Patch& other) const;
    bool operator<(const Patch& other) const;
};

struct Hamming {
    struct Cfg {
        Cfg(Patch origin, const std::string& search, unsigned maxdiff);
        Cfg(Patch origin, const std::string& search, float maxd_ratio = 0.125f);

        Patch token; // backreference to needle origin
        std::string needle;
        unsigned threshold;
    };

    enum State {
        Incomplete = 0,
        Matched,
        Failed,
    };

    Hamming(const Cfg& cfg, std::streamoff pos);
    State suggest(char c);

    Cfg config;
    const std::streamoff base;
    unsigned next_index;
    unsigned difference;
};

struct Slicer {
    struct Result {
        std::streamoff pos;
        unsigned difference;
        float confidence;
    };

    Slicer(std::streamoff pos = 0u);
    void search(const Hamming::Cfg& predicate);

    void suggest(std::streamoff pos, char c); // handtracking
    void suggest(char c);

    void analyze(std::istream& is);
    void stuff(std::ostream& os, std::function<const char*(const Patch&)> rpl) const;

    std::list<Hamming::Cfg> watch;
    std::streamoff next_pos;
    std::list<Hamming> knives;
    std::multimap<Patch, Result> found_log;
    std::multimap<std::string, Result> found_lit;

private:
    void post(const Hamming& hamming);
};

} // namespace mod

#endif
