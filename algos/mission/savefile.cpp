#include "savefile.h"

#include "memory.h"

namespace dat {

SaveFile::SaveFile() {
    *this = {}; // equivalent to memset(this, 0, sizeof(*this)) but respects RTTI
}

} // namespace dat