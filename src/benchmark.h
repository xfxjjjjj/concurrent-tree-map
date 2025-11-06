#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <thread>
#include <vector>
#include <random>    // For random workload generation
#include <optional>  // For tree_map.get() return type
#include <stdexcept> // For error handling

// --- Data Types and Base Class ---
// Define the byte array type for keys and values
// Include the base class for your tree map
#include "src/tree_map_base.h" // Assumes TreeMapBase<K, V> is defined here

namespace benchmark {

// --- Global Constants ---
// Define constants used to configure the concurrent workload and key generation.
const size_t KEY_SIZE = 16;      // Fixed size (in bytes) for keys/values
const size_t MAX_KEY = 1000000;  // Range of integer IDs used to generate keys (shared key space)

// --- Helper Structs ---

// Structure to collect detailed statistics for each thread
struct ThreadStats {
    size_t puts_performed = 0;
    size_t gets_performed = 0;
    size_t successful_gets = 0;
    size_t failed_gets = 0;
};

// --- Helper Function Declarations ---

// Serializes a size_t ID into a fixed-size ByteVector key/value.
ByteArray create_key(size_t id);

// --- Core Benchmark Function Declarations ---

// The function executed by each thread, implementing the mixed put/get workload.
void ThreadBody(TreeMapBase& tree_map,
                size_t num_operations,
                double put_ratio,
                size_t thread_id,
                ThreadStats& stats);

// --- Main Benchmark Runner (Template Definition) ---

// Template function to run the benchmark with any TreeMap type.
// The full definition is kept here as per C++ template best practices.
template <typename TreeMapType>
int RunBenchmark(const int argc, char** argv) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0]
                  << " num_threads initial_size ops_per_thread put_ratio" << std::endl;
        return 1;
    }

    // Argument Parsing
    size_t num_threads = std::stoul(std::string(argv[1]));
    size_t initial_size = std::stoul(std::string(argv[2]));
    size_t ops_per_thread = std::stoul(std::string(argv[3]));
    double put_ratio = std::stod(std::string(argv[4]));

    if (put_ratio < 0.0 || put_ratio > 1.0) {
        std::cerr << "Error: put_ratio must be between 0.0 and 1.0" << std::endl;
        return 1;
    }

    // --- 1. Initialization and Pre-population (Sequential) ---
    TreeMapType tree_map;

    std::cout << "--- Pre-populating tree with " << initial_size << " elements... ---" << std::endl;
    const auto pre_populate_start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < initial_size; ++i) {
        ByteArray key = create_key(i);
        tree_map.put(key, key);
    }
    const auto pre_populate_end = std::chrono::high_resolution_clock::now();
    const auto pre_populate_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(pre_populate_end - pre_populate_start).count();
    std::cout << "Pre-population took: " << pre_populate_ms << " ms" << std::endl;

    // --- 2. Concurrent Execution Phase ---
    std::vector<ThreadStats> all_stats(num_threads);
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    const size_t total_operations = num_threads * ops_per_thread;
    std::cout << "--- Starting Concurrent Mixed Workload (" << num_threads << " threads) ---" << std::endl;
    std::cout << "Total operations: " << total_operations << " ("
              << (put_ratio * 100) << "% PUTs, "
              << ((1.0 - put_ratio) * 100) << "% GETs)" << std::endl;

    const auto begin_time = std::chrono::high_resolution_clock::now();
    // In benchmark::RunBenchmark, line ~101:
    for (size_t i = 0; i < num_threads; i++) {
        // Create a local copy of the loop index for the thread
        size_t thread_index = i;

        threads.emplace_back(std::thread(ThreadBody,
                                         std::ref(tree_map),
                                         ops_per_thread,
                                         put_ratio,
                                         thread_index,
                                         std::ref(all_stats.at(i))));
    }
    for (auto& thread : threads) {
        thread.join();
    }
    const auto end_time = std::chrono::high_resolution_clock::now();

    // --- 3. Results and Validation ---
    size_t total_puts = 0;
    size_t total_gets = 0;
    size_t total_success_gets = 0;
    for (const auto& stats : all_stats) {
        total_puts += stats.puts_performed;
        total_gets += stats.gets_performed;
        total_success_gets += stats.successful_gets;
    }

    const auto duration = end_time - begin_time;
    const auto total_ms = static_cast<double>
        (std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());

    const size_t total_operations_performed = total_puts + total_gets;

    std::cout << "\n--- Benchmark Results ---" << std::endl;
    std::cout << "Concurrent computation took: " << total_ms << " ms" << std::endl;
    std::cout << "Total operations performed: " << total_operations_performed << std::endl;
    // Calculate Throughput (Operations/second)
    std::cout << "Throughput (Ops/sec): "
              << static_cast<double> (total_operations_performed) / (total_ms / 1000.0) << std::endl;
    std::cout << "\nOperation Breakdown:" << std::endl;
    std::cout << "  Total PUTs: " << total_puts << std::endl;
    std::cout << "  Total GETs: " << total_gets << std::endl;
    std::cout << "  Successful GETs: " << total_success_gets << std::endl;

    std::cout << argv[0] << " succeeded" << std::endl;
    return 0;
}

}  // namespace benchmark

#endif // BENCHMARK_H
