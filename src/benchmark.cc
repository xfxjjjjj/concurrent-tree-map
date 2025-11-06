#include "src/benchmark.h"
#include <random>
#include <optional>
#include <cstring>

namespace benchmark {
    // --- Global Helper Function Definitions ---

    // Definition of the key serialization function
    ByteArray create_key(size_t id) {
        // Ensure we don't try to copy more bytes than the key size
        constexpr size_t bytes_to_copy = std::min(sizeof(id), KEY_SIZE);

        // 1. Initialize the ByteArray with KEY_SIZE elements (all zero)
        ByteArray key(KEY_SIZE, std::byte{0});

        // 2. Get a pointer to the raw bytes of the size_t ID
        const auto* id_bytes = reinterpret_cast<const std::byte*>(&id);

        // 3. Copy the ID's bytes into the start of the ByteArray
        // Note: This copy is typically little-endian on most architectures.
        std::memcpy(key.data(), id_bytes, bytes_to_copy);

        return key;
    }

    // --- ThreadBody Function Definition ---

    // Implementation of the thread's mixed workload logic
    void ThreadBody(TreeMapBase& tree_map,
                    size_t num_operations,
                    double put_ratio,
                    size_t thread_id,
                    ThreadStats& stats) {

        // Reset stats
        stats = ThreadStats{};

        // Use a thread-local random generator
        std::mt19937 gen(thread_id + std::random_device{}());
        std::uniform_real_distribution<> op_dist(0.0, 1.0);
        std::uniform_int_distribution<size_t> key_dist(0, MAX_KEY - 1);

        for (size_t i = 0; i < num_operations; ++i) {
            size_t key_id = key_dist(gen);
            ByteArray key = create_key(key_id);
            ByteArray value = create_key(key_id);

            if (op_dist(gen) < put_ratio) {
                // PUT Operation
                tree_map.put(key, value);
                stats.puts_performed++;
            } else {
                // GET Operation
                std::optional<ByteArray> result = tree_map.get(key);
                stats.gets_performed++;
                if (result.has_value()) {
                    stats.successful_gets++;
                } else {
                    stats.failed_gets++;
                }
            }
        }
    }
}
