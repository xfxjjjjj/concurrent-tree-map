# Concurrent Tree-based Map

## Red Black Tree:

**structures, helpers**: src/tree_map_{lock_level}.h

**operation Implementations**: src/tree_map_{lock_level}.cpp

## lock_level:

I've implemented coarse grained lock which achieves synchronization via a global mutex,

and a fine-grained one that is still under debugging

## Benchmark:

To view the benchmark results, I've triggered github actions running exactly ./scripts/run_benchmark.sh

## System Requirements: 

clang-18, clang++-18, lvm-18
cmake, libc++-18 libc++-18-dev

