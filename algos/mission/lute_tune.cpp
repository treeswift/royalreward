#include "lute_tune.h"

#include "dat_defs.h"

#include <cstring>
#include <vector>

namespace {
    using namespace map;
}

// Lute tunes, or LUT tuning.

namespace loc {

constexpr const char conts[dat::kAlphabet] = {0, 1, 0, 1, 2, 0, 2, 2, 0, 2, 0, 2, 1,
                                              0, 0, 0, 1, 0, 3, 2, 3, 0, 0, 2, 1, 3};
constexpr const char fortx[dat::kAlphabet + 2] = "\x1e\x2f\x24\x1e\xb\x16\x29\x2b\xb\x29\x39\x34\x19"
                                                 "\x16\6\x3a\x2a\x36\x11\x09\x29\x28\x28\x2d\x13\x2e\x0b";
constexpr const char forty[dat::kAlphabet + 2] = "\x1b\6\x31\x12\x2e\x31\x24\x1b\x1e\x22\x3a\x39"
                                                 "\x27\x18\x39\x17\x38\6\x27\x12\x0c\5\x29\6\x13\x2b\7";

constexpr const char ptofs[dat::kAlphabet + 1] = "\x3\xe\x7\x10\xc\x16\x12\x15\x11\x13\xd\x5\x9\xf\xb\x2\x8\0\x6\x4\x1\x14\x18\xa\x17\x19";

const char* letters[4] = {
    // shuffling changes names of of specific<x,y> fort/port pair
    "VACFIKNOPRW", // keep V first to make H the first port ever
    "BDJMQY",
    "EGHLTX",
    "SUZ",
};

                                                // "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
constexpr const char mapping[dat::kAlphabet + 1] = "DOHQMWSVRTNFJPLCIAGEBUYKXZ";

struct Port {
    Port(const Point& base, const Shift& to_bay, const Shift& to_air)
        : loc(base), bay(base + to_bay), air(base + to_air) {} 

    Point loc;
    Point bay;
    Point air;
};

struct Fort {
    Fort(const Point& f, const Port& p, char c = -1)
        : loc(f), port(p), cont(c) {}

    char cont;
    Point loc;
    Port port;
};

struct Land {
    Land();

    std::vector<Fort> forts;
};

const Shift nn = { 0, 1}, ss = { 0,-1};
const Shift ww = {-1, 0}, ee = { 1, 0};
const Shift nw = nn + ww, ne = nn + ee;
const Shift sw = ss + ww, se = ss + ee;

Land::Land() : forts
    {
        {{30,	27},	{{29,	12},	{ 1,	-1},	{ 0,	-1}}},
        {{47,	6}, 	{{58,	4},		{ 1,	 1},	{-1,	 0}}},
        {{36,	49},	{{38,	50},	{ 1,	 0},	{ 0,	-1}}},
        {{30,	18},	{{34,	23},	{ 0,	-1},	{ 1,	 0}}},
        {{11,	46},	{{5,	50},	{-1,	-1},	{ 0,	-1}}},
        {{22,	49},	{{17,	44},	{ 1,	 0},	{-1,	 0}}},
        {{41,	36},	{{13,	60},	{ 1,	 0},	{-1,	 0}}},
        {{43,	27},	{{9,	39},	{ 1,	-1},	{ 0,	-1}}},
        {{11,	30},	{{14,	27},	{ 0,	-1},	{-1,	 0}}},
        {{41,	34},	{{58,	33},	{ 0,	-1},	{-1,	 0}}},
        {{57,	58},	{{51,	28},	{ 0,	-1},	{ 0,	 1}}},
        {{52,	57},	{{57,	57},	{ 1,	-1},	{ 0,	-1}}},
        {{25,	39},	{{3,	37},	{-1,	-1},	{ 0,	-1}}},
        {{22,	24},	{{17,	21},	{ 1,	 0},	{-1,	 0}}},
        {{6, 	57},	{{41,	58},	{ 0,	-1},	{-1,	 0}}},
        {{58,	23},	{{50,	13},	{-1,	-1},	{ 0,	 1}}},
        {{42,	56},	{{58,	60},	{ 1,	 0},	{ 0,	-1}}},
        {{54,	6}, 	{{57,	5},		{-1,	-1},	{-1,	 0}}},
        {{17,	39},	{{9,	60},	{ 1,	 0},	{ 0,	-1}}},
        {{9, 	18},	{{13,	7},		{-1,	 0},	{ 0,	 1}}},
        {{41,	12},	{{7,	3},		{ 1,	 0},	{-1,	 0}}},
        {{40,	5}, 	{{12,	3},		{-1,	 0},	{ 0,	 1}}},
        {{40,	41},	{{46,	35},	{ 1,	 1},	{ 1,	 0}}},
        {{45,	6}, 	{{49,	8},		{ 1,	 1},	{ 1,	 0}}},
        {{19,	19},	{{3,	8},		{-1,	 1},	{ 0,	 1}}},
        {{46,	43},	{{58,	48},	{ 1,	 0},	{ 0,	-1}}},
    }
{
    ;
};

const char* fort_letters(unsigned c_index) {
    return letters[c_index];
}

void old_tune(dat::Leftovers& lovers) {
    constexpr unsigned kA = dat::kAlphabet;
    std::memcpy(lovers.conts, conts, kA);
    std::memcpy(lovers.forts[0], fortx, kA + 1);
    std::memcpy(lovers.forts[1], forty, kA + 1);
    // TODO havens, airfields, bays
    std::memcpy(lovers.ptofs, ptofs, kA);
}

} // namespace loc
