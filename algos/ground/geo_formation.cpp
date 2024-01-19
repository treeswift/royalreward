#include "geography.h"

namespace map {


void Continent::stroke(const Point& p0, const Point& p1, char color) {
    minerals.push_back({p0, color});

    for(Real f = 0.f; f < 1.f; f += (1.f / kMapDim)) {
        Point p = p0.blend(p1, f);
        (p == minerals.back()) || (minerals.push_back({p, color}), true);
    }
}

void Continent::tectonics() {
    Block sbox = bound(kMinBox, kMinBox + kFeaAmp);
    Point base = corner(0);
    Shift dmax = diag(kMapDim - kMinBox);

    for(unsigned fl = 0; fl < kNLines; ++fl) {
        Shift s = sbox.rand() - base;
        Point d = base + (dmax - s).rand();

        Point p0 = d + s.rand();
        Point p1 = d + s.rand();

        char color = rnd::upto(kMaxCol) + 1u;
        stroke(p0, p1, color);
    }

    // Primordial Earth
    if(kGround != kMature) {
        for(auto itr = minerals.begin(); itr != minerals.end(); ) {
            if(major.covers(*itr) && (itr->color != kGround)) {
                itr = minerals.erase(itr);
            } else {
                ++itr;
            }
        }
        minerals.push_back({entry, kGround});
        minerals.push_back({ruler, kGround});
        if(kWizard != kMature) {
            minerals.push_back({magic, kWizard});
        }
    }
}

void Continent::formLand() {
    tectonics();

    struct Allegiance {
        Real part[kColors];

        Allegiance() { reset(); }

        void reset() { std::fill(part, part + kColors, 0.f); }
    };

    conti.visit([&]WITH_XY{
        Allegiance all;
        for(const Paint& f : minerals) {
            int dx = f.x - x;
            int dy = f.y - y;
            unsigned color = f.color;
            unsigned dcolor = color ? color : kMaxCol + 1;
            Real decay = kDecay + dcolor * (kDPow2 * dcolor + kDPow1); // Horner
            // color *= color & 1u; // more water but fewer islands!
            Real rbf = (dx * dx + dy * dy);
            Real prominence = 1.f; // decay, etc.
            all.part[color] += prominence * expf(- decay * rbf);
        }
        unsigned idx = 0; // guaranteed to be overwritten
        Real max = 0.f;
        Real sum = 0.f;
        for(unsigned color = 0; color <= kMaxCol + 1; ++color) {
            Real pt = all.part[color];
            sum += pt;
            if(all.part[color] > max) {
                max = pt;
                idx = color;
            }
        }
        // torn edge: make winning harder near water
        Real edge_d = std::min(
            std::min(x, kMapMax - x), 
            std::min(y, kMapMax - y)
        );

        Real lns = std::log10(sum + kRoughn) * kSmooth;
        if(kGround != kMature && x <= ruler.x && y == edge_d) {
            // 10-11,3 water; 12-... land
            lns = (x & 2) ? -3 : -2;
        }
        bool edge_cond = (lns + edge_d > kRugged);

        if(max * kWinner > sum && edge_cond) { // majority winner
            map[y][x] = cWater + idx;
        }
    });
}

void Continent::segregate() {
    MapHolder<char> chrout{chrmem};
    ChrMap& chm = chrout.map();
    conti.visit([&]WITH_XY{
        char color = chm[y][x];
        auto segregate = [&](unsigned xo, unsigned yo) {
            if(color != cWater) {
                char ocolor = chm[yo][xo];
                if(ocolor != cWater && color != ocolor) {
                    map[y][x] = cWater; // leave <xo, yo>
                }
            }
        };
        segregate(x + 1, y);
        segregate(x + 1, y + 1);
        segregate(x,     y + 1);
    });
}


} // namespace map