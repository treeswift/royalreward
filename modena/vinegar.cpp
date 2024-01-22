#include "vinegar.h"

#include <boost/filesystem.hpp>

namespace mod {

namespace bs = boost::system;
namespace fs = boost::filesystem;

constexpr const char* kCacheDir = "Modena";

#define EXTENSION "\x2e\x45\x58\x45"
#define TRANSIENT "\x2e\x55\x4e\x50"
#define SAVEDFILE "\x2e\x44\x41\x54"
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

bool soak(Salad& salad, const fs::path& base, const fs::path& out) {
    //
    return false;
}

Salad::Salad(const std::string& dir) : path(dir), nutrients{} {
    fs::path base(path);
    bs::error_code errc;
    if(fs::is_directory(base)) {
        progress = Dir_Found;
        // we are doing case-insensitive search on possibly case-sensitive FS:
        for(const fs::directory_entry& entry : fs::directory_iterator(base)) {
            if(fs::is_regular_file(entry)) {
                list_nutrient(entry.path().filename().generic_string());
            }
        }
        fs::path cache = base / kCacheDir;
        if(fs::create_directories(cache, errc), !errc) {
            progress = Has_Cache;
            fs::path marinated = cache / kU2;
            if(fs::is_regular_file(marinated) || soak(*this, base, marinated)) {
                progress = Marinated;
            }
        }
    }
}

} // namespace mod
