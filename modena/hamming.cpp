#include "hamming.h"

#include "mumbling.h"

namespace mod {

Hamming::Cfg::Cfg(Token origin, const std::string& search, float ratio)
    : Hamming::Cfg::Cfg(origin, search, (unsigned) (ratio * search.size())) {}

Hamming::Cfg::Cfg(Token origin, const std::string& search, unsigned max_diff)
    : token(origin), needle(search), threshold(max_diff) {}

Hamming::Hamming(const Cfg& cfg, std::streamoff pos)
    : config(cfg), base(pos), next_index(0), difference(0) {}

Hamming::State Hamming::suggest(char c) {
    difference += c != config.needle.at(next_index++);
    return (difference > config.threshold) ? State::Failed
        : (next_index == config.needle.size()) ? State::Matched
        : State::Incomplete;
}

Slicer::Slicer(std::streamoff pos) : next_pos(pos) {}

void Slicer::search(const Hamming::Cfg& predicate) {
    watch.push_back(predicate);
}

void Slicer::suggest(std::streamoff pos, char c) {
    if(pos != next_pos) {
        mum::bummer<std::logic_error>("pos (%ld) != expected (%ld)", pos, next_pos);
    }
    suggest(c);
}

void Slicer::suggest(char c) {
    auto itr = knives.begin();
    // test all existing
    while(itr != knives.end()) {
        auto state = itr->suggest(c);
        if(Hamming::State::Matched == state) {
            post(*itr);
        }
        if(Hamming::State::Incomplete == state) {
            ++itr;
        } else {
            // Matched or Failed
            itr = knives.erase(itr);
        }
    }
    // insert hopefuls
    for(const auto& pred : watch) {
        // we can use itr, but it's always knives.end()
        knives.push_back(Hamming{pred, next_pos});
    }
    // step next
    ++next_pos;
}

void Slicer::post(const Hamming& hamming) {
    unsigned size = hamming.config.needle.size();
    float confidence = 1.f * (size - hamming.difference) / size;
    Result result{hamming.base, hamming.difference, confidence};
    found_log.insert({hamming.config.token, result});
    found_lit.insert({hamming.config.needle, result});
}

}
