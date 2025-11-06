#include "src/benchmark.h"

#include <cstring>
#include <optional>
#include <random>

namespace benchmark {
// --- Global Helper Function Definitions ---

// Definition of the key serialization function
ByteArray create_key(const size_t id) {
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

void ThreadBody(
    TreeMapBase& tree_map,  // Changed from TreeMapBase<K,V> to the
                            // non-templated base class
    size_t num_operations,  // Total operations per thread
    double put_ratio,       // Ratio of PUTs (e.g., 0.3 for 30% puts, 70% gets)
    size_t thread_id, ThreadStats& stats) {
  // Reset stats
  stats = ThreadStats{};

  // Use a thread-local random generator
  std::mt19937 gen(thread_id + std::random_device{}());
  std::uniform_real_distribution<> op_dist(0.0, 1.0);
  std::uniform_int_distribution<size_t> key_dist(0, MAX_KEY - 1);

  for (size_t i = 0; i < num_operations; ++i) {
    const size_t key_id = key_dist(gen);
    const ByteArray key = create_key(key_id);
    const ByteArray value = create_key(key_id);

    // Decide between PUT and GET
    if (op_dist(gen) < put_ratio) {
      // --- PUT Operation (Write) ---
      tree_map.put(key, value);
      stats.puts_performed++;  // Track total puts (which is used to estimate
                               // size)
    } else {
      // --- GET Operation (Read) ---
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
}  // namespace benchmark
