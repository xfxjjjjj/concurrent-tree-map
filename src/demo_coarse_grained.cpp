#include "src/benchmark.h"
#include "src/tree_map_coarse_grained.h"

int main(int argc, char** argv) {
    return benchmark::RunBenchmark<TreeMapCoarseGrained> (argc, argv);
}
