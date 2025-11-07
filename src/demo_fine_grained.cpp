#include "src/benchmark.h"
#include "src/tree_map_fine_grained.h"

int main(int argc, char** argv) {
    return benchmark::RunBenchmark<TreeMapFineGrained> (argc, argv);
}
