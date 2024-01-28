#include "vinegar.h"
#include "mumbling.h"

#include <boost/filesystem.hpp>

namespace upack {
extern int main(int argc, char** argv);
}

namespace ucomp {
extern int main(int argc, char** argv);
}

namespace mod {

namespace bs = boost::system;
namespace fs = boost::filesystem;

constexpr const char* kCacheDir = "Modena.mod";

#define EXTENSION "\x2e\x45\x58\x45"
#define TRANSIENT "\x2e\x55\x50\x4b"
#define SAVEDFILE "\x2e\x44\x41\x54"
#define OLDERFILE "\x2e\x44\x41\x54"
constexpr const char* kU0 = "\x4b\x42" EXTENSION;
constexpr const char* kU1 = "\x55\x31" TRANSIENT;
constexpr const char* kU2 = "\x55\x32" TRANSIENT;

std::string unicase(std::string s) {
    // the case choice doesn't really matter,
    // but D0580x displays names in uppercase
    for(auto& c : s) {
        c = std::toupper(c);
    }
    return s;
}

bool isdat(const std::string& si) {
    return si.size() >= 4 && (si.substr(si.size()-4, 4) == SAVEDFILE);
}

bool isndat(const std::string& si) {
    return !isdat(si);
}

void Salad::list_nutrient(const std::string& case_preserved) {
    std::string case_erased = unicase(case_preserved);
    if(isndat(case_erased)) {
        nutrients[unicase(case_preserved)] = case_preserved;
    }
}

bool soak(const fs::path& k0, const fs::path& woods, const fs::path& out) {
    fs::path k1 = woods / kU1;
    std::string s0 = k0.generic_string();
    std::string s1 = k1.generic_string();
    char* args[3] = {nullptr, const_cast<char*>(s0.c_str()), const_cast<char*>(s1.c_str())};
    if(ucomp::main(3, args)) { // already uncomp'ed
        k1 = k0;
        s1 = s0;
    }

    std::string s2 = out.generic_string();
    args[1] = const_cast<char*>(s1.c_str());
    args[2] = const_cast<char*>(s2.c_str());
    if(upack::main(3, args)) { // already unpack'ed
        bs::error_code errc;
        fs::remove(out);
        fs::copy_file(k1, out, errc);
        return !errc;
    }
    return true;
}

bool soak(Salad& salad, const fs::path& woods, const fs::path& out) {
    auto nut = salad.nutrients.find(kU0);
    if(salad.nutrients.end() == nut) return false;

    fs::path k0 = woods / nut->second;
    return soak(k0, woods, out);
}

std::string unpack(const std::string& packed) {
    fs::path woods = fs::temp_directory_path();
    fs::path k0{packed};
    fs::path k2 = woods / kU2;
    std::string output = k2.generic_string();
    return soak(k0, woods, k2) ? output : packed;
}

Salad::Salad(const std::string& dir) : path(dir), nutrients{} {
    fs::path woods(path);
    bs::error_code errc;
    if(fs::is_directory(woods)) {
        progress = Dir_Found;
        // we are doing case-insensitive search on possibly case-sensitive FS:
        for(const fs::directory_entry& entry : fs::directory_iterator(woods)) {
            if(fs::is_regular_file(entry)) {
                list_nutrient(entry.path().filename().generic_string());
            }
        }
        fs::path cache = woods / kCacheDir;
        if(fs::create_directories(cache, errc), !errc) {
            progress = Has_Cache;
            fs::path marinated = cache / kU2;
            if(fs::is_regular_file(marinated) || soak(*this, woods, marinated)) {
                progress = Marinated;
            }
        }
    }
}

std::string  Salad::make(const dat::SaveFile& sf, const mod::Leftovers& lovers) {
    // extract name correction and search
    std::string fname{sf.name, sf.name+8};
    while(' ' == fname.back()) fname.pop_back();
    // TODO prevent world name collision with cache dir
    
    constexpr auto kWrite = std::ios_base::out | std::ios_base::binary;
    constexpr auto kAmend = std::ios_base::in | kWrite;
    fs::path woods = fs::path(path);
    fs::path world = woods / fname;
    fs::path towne = woods / kCacheDir;
    bs::error_code errc;
    fs::create_directories(world, errc);
    fs::path quest = world / (fname + SAVEDFILE);
    fs::path older = world / (fname + OLDERFILE);
    {
        fs::remove(older, errc);
        fs::rename(quest, older, errc);
        std::fstream qfs{quest.generic_string(), kWrite};
        qfs.write(sf.name, sizeof(sf));
        qfs.flush();
    }
    for(const auto& nut : nutrients) {
        bool kproper = nut.first == kU0;
        fs::path dst = world / nut.first;
        fs::path src = woods / nut.second;
        if(kproper) {
            src = towne / kU2;
            fs::remove(dst, errc);
        }
        fs::copy_file(src, dst, errc);
        // TODO ... &= !errc.value()
        if(kproper) {
            std::fstream fix{dst.generic_string(), kAmend};
            Leftovers ref;
            old_tune(ref);
            Slicer slicer;
            Margins legend; // TOOD make optional
            ref.sharpen(slicer, legend);
            slicer.analyze(fix);
            slicer.stuff(fix, [&](const Patch& patch) {
                switch(patch.origin) {
                    case Origin::LUT:
                        return reinterpret_cast<const char*>(&lovers) + patch.selector;
                    case Origin::STR:
                    default:
                        return reinterpret_cast<const char*>(patch.selector);
                }
            });
            fix.flush();
        }
    }
    return fname;
}

} // namespace mod
