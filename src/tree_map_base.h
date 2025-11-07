
#ifndef UNTITLED_TREE_MAP_BASE_H
#define UNTITLED_TREE_MAP_BASE_H
#include <algorithm>
#include <cstddef>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <utility>
#include <vector>

using ByteArray = std::vector<std::byte>;

enum Color { RED, BLACK };

struct TreeNode : public std::enable_shared_from_this<TreeNode> {
  Color color_ = RED;
  ByteArray key_;
  ByteArray value_;
  std::shared_ptr<TreeNode> left_;
  std::shared_ptr<TreeNode> right_;
  std::weak_ptr<TreeNode> parent_;  // Changed to weak_ptr

  std::shared_mutex lock_;

  explicit TreeNode(ByteArray key, ByteArray value)
      : key_{std::move(key)}, value_{std::move(value)} {}

  // Helper method to get parent as shared_ptr (or null if expired)
  std::shared_ptr<TreeNode> get_parent() { return parent_.lock(); }
};

inline bool is_less(const ByteArray& a, const ByteArray& b) {
  return std::ranges::lexicographical_compare(a, b);
}

inline bool is_equal(const ByteArray& a, const ByteArray& b) {
  return std::ranges::equal(a, b);
};

class TreeMapBase {
 public:
  virtual ~TreeMapBase();

  // Adds |key| to the hash set. Returns true if |elem| was absent, and false
  // otherwise.
  virtual void put(ByteArray key, ByteArray value) = 0;

  // Returns true if |key| is present in the hash set, and false otherwise.
  virtual std::optional<ByteArray> get(const ByteArray& key) = 0;
};

#endif  // UNTITLED_TREE_MAP_BASE_H
