#include "geology.h"

namespace map {

Geology::Geology(int elements) {
    setComposition(elements);
}

void Geology::setComposition(int elements) {
    kMaxCol = elements;

    // if the following values have been applied to kGround and kWizard, the latter need to be recalculated
    // for now, we simply maintain the order. (way to fix: add the boolean flag "kMaxCol-based" to elements)

    kIsland = kMaxCol + 1;
    kHermit = kMaxCol + 2;
}

} // namespace map