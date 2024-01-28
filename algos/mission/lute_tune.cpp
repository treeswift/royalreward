#include "lute_tune.h"

#include "dat_defs.h"

#include <string>
#include <vector>

namespace {
    using namespace map;
}

// Lute tunes, or LUT tuning.

namespace loc {

                                              // "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
constexpr const char ptofs[dat::kAlphabet + 1] = "DOHQMWSVRTNFJPLCIAGEBUYKXZ";

struct Port {
    Port(const Point& base, const Shift& to_bay, const Shift& to_air)
        : loc(base), bay(base + to_bay), air(base + to_air) {} 

    Port() = default; // no-port

    Point loc;
    Point bay;
    Point air;
};

struct Fort {
    Fort(const Point& f, const Port& p, char c = -1)
        : loc(f), port(p), cont(c) {}

    Point loc;
    Port port;
    char cont;
};

const struct Land {
    Land();

    std::vector<Fort> forts;
    std::vector<std::string> ctofs;
} land;

const Shift nn = { 0, 1}, ss = { 0,-1};
const Shift ww = {-1, 0}, ee = { 1, 0};
const Shift nw = nn + ww, ne = nn + ee;
const Shift sw = ss + ww, se = ss + ee;

Land::Land() : forts
    {
        {{30,	27},	{{29,	12},	se,	ss}},
        {{47,	6}, 	{{58,	4},		ne,	ww}},
        {{36,	49},	{{38,	50},	ee,	ss}},
        {{30,	18},	{{34,	23},	ss,	ee}},
        {{11,	46},	{{5,	50},	sw,	ss}},
        {{22,	49},	{{17,	44},	ee,	ww}},
        {{41,	36},	{{13,	60},	ee,	ww}},
        {{43,	27},	{{9,	39},	se,	ss}},
        {{11,	30},	{{14,	27},	ss,	ww}},
        {{41,	34},	{{58,	33},	ss,	ww}},
        {{57,	58},	{{51,	28},	ss,	nn}},
        {{52,	57},	{{57,	57},	se,	ss}},
        {{25,	39},	{{3,	37},	sw,	ss}},
        {{22,	24},	{{17,	21},	ee,	ww}},
        {{6, 	57},	{{41,	58},	ss,	ww}},
        {{58,	23},	{{50,	13},	sw,	nn}},
        {{42,	56},	{{58,	60},	ee,	ss}},
        {{54,	6}, 	{{57,	5},		sw,	ww}},
        {{17,	39},	{{9,	60},	ee,	ss}},
        {{9, 	18},	{{13,	7},		ww,	nn}},
        {{41,	12},	{{7,	3},		ee,	ww}},
        {{40,	5}, 	{{12,	3},		ww,	nn}},
        {{40,	41},	{{46,	35},	ne,	ee}},
        {{45,	6}, 	{{49,	8},		ne,	ee}},
        {{19,	19},	{{3,	8},		nw,	nn}},
        {{46,	43},	{{58,	48},	ee,	ss}},
        {{11,	7},     {}} // Washington, D.C.
    },
    ctofs{
        // swapping letters swaps names between fort/port pairs
        "VACFIKNOPRW", // keep V first to make H the first port
        "BDJMQY",
        "EGHLTX",
        "SUZ",
    }
{
    unsigned cont = 0;
    for(const auto& fs : ctofs) {
        for(char f : fs) {
            unsigned fort = f - 'A';
            forts.at(fort).cont = cont;
        }
        ++cont;
    }
}

const char* fort_letters(unsigned c_index) {
    return land.ctofs.at(c_index).c_str();
}

void old_tune(dat::Leftovers& lovers) {
    constexpr unsigned kA = dat::kAlphabet;
    for(unsigned fid = 0; fid <= kA; ++fid) { // sic; we want to include the D.C.
        // fort proper
        lovers.forts[0][fid] = land.forts.at(fid).loc.x;
        lovers.forts[1][fid] = land.forts.at(fid).loc.y;
    }
    for(unsigned pid = 0; pid < kA; ++pid) {
        lovers.conts[pid] = land.forts.at(pid).cont; // fort and port share cont
        // port proper
        const Port& port = land.forts.at(pid).port;
        lovers.ports[0][pid] = port.loc.x;
        lovers.ports[1][pid] = port.loc.y;
        // sea travel
        lovers.p_bay[0][pid] = port.bay.x;
        lovers.p_bay[1][pid] = port.bay.y;
        // air travel
        lovers.p_air[0][pid] = port.air.x;
        lovers.p_air[1][pid] = port.air.y;
        lovers.ptofs[pid] = ptofs[pid] - 'A';
    }
}

} // namespace loc
