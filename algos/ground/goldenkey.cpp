#include "goldenkey.h"

namespace map {

char GoldenKey::convert(char c) {
    if(c >= cCRear && c <= cCRear + kCastles) {
        c = cCRear;
    }
    else switch(c) { // replace w/a LUT?
        case cPaper:
        case cGlass:
        case cGift1:
        case cGift2:
        case cAddMe:
        case cChest:
        case cEnemy:
            c = cPlain;
    }
    return c;
}

std::string GoldenKey::look(const ChrMap& map, const Point& p) {
    std::string hash;
    hash.reserve(25);
    screen(p).visit([&]WITH_XY {
        char c = convert(map[y][x]);
        // TODO add options to bury the key under cSands, cLabel or cEntry
        hash.push_back(c); // (25+ovh)*4*64*64 >= 400k... may want to pack
    });
    return hash;
}

void GoldenKey::consider(const Continent& cont) {
    const auto& map = cont.map;
    conti = cont.conti; // FIXME make static in Continent
    conti.visit([&]WITH_XY {
        spot.p = Point{x, y};
        char c = convert(map[y][x]);
        if(cPlain == c) {
            stats_locs++;
            std::string key = look(map, spot.p);
            auto ins = papermaps.insert({key, spot});
            ins.first->second.ucount++;
            bool& uniq = ins.first->second.unique;
            if(ins.second) {
                uniq = true;
                stats_puts++;
            } else {
                stats_rejc += uniq;
                uniq = false;
            }
        }
    });
    maps.push_back(&map);
    spot.cindex++;
}

GoldenKey::Burial GoldenKey::select() const {
    if(stats_puts <= stats_rejc) {
        for(const auto& k : papermaps) {
            fprintf(stderr, "%s [%u]\n", k.first.c_str(), k.second.ucount);
        }
        fprintf(stderr, "No unique map locations! %lu=%u?=%u+%u\n", papermaps.size(), stats_locs, stats_puts, stats_rejc);
        // std::abort();
        return spot;
    }
    unsigned attemptcount = 32; // more than enough, but given the fallback is brute force iteration, err on the safe side
    while(attemptcount > 0) {
        unsigned cindex = rnd::upto(spot.cindex);
        const ChrMap& map = *(maps.at(cindex));
        Point p = conti.rand();
        std::string hash = look(map, p);
        auto itr = papermaps.find(hash);
        if(itr != papermaps.cend() && itr->second.unique) {
            return spot = itr->second;
        }
        --attemptcount;
    }
    // brewt force
    unsigned pick = rnd::upto(stats_puts - stats_rejc);
    unsigned item = 0;
    for(auto itr = papermaps.cbegin(); itr != papermaps.cend(); ++itr) {
        if(item == pick) {
            return itr->second;
        }
        item += itr->second.unique;
    }
    fprintf(stderr, "No unique location found! %lu=%u?=%u+%u\n", papermaps.size(), stats_locs, stats_puts, stats_rejc);
    // std::abort();
    return spot;
}

} // namespace map
