#include <src/tree_map_coarse_grained.h>

#include "src/benchmark.h"

namespace check_coarse_grained {

    void Placeholder();

    void Placeholder() {
        TreeMapCoarseGrained hm{};
        hm.put(1, 1);
        (void)hm.get(1);
    }

}  // namespace check_coarse_grained
