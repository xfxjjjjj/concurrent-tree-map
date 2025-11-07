#ifndef HASHSETS_TREE_MAP_FINE_GRAINED_H
#define HASHSETS_TREE_MAP_FINE_GRAINED_H
#include <src/tree_map_base.h>

#include <mutex>

class TreeMapFineGrained final : public TreeMapBase {
 public:
  // ... constructor and members ...
  void put(ByteArray key, ByteArray value) override;
  std::optional<ByteArray> get(const ByteArray& key) override;

 private:
  std::shared_mutex root_lock_;
  std::shared_ptr<TreeNode> root{nullptr};
  void left_rotate(std::shared_ptr<TreeNode> x);
  void right_rotate(std::shared_ptr<TreeNode> y);
  void rbt_insert_fixup(std::shared_ptr<TreeNode> cur);
};

// Helper function to lock multiple nodes in address order
inline void lock_nodes(std::vector<std::shared_ptr<TreeNode>>& nodes) {
  // Sort by address to prevent deadlock
  std::sort(
      nodes.begin(), nodes.end(),
      [](const std::shared_ptr<TreeNode>& a,
         const std::shared_ptr<TreeNode>& b) { return a.get() < b.get(); });

  // Lock in sorted order
  for (auto& node : nodes) {
    node->lock_.lock();
  }
}

// Helper function to unlock multiple nodes
inline void unlock_nodes(std::vector<std::shared_ptr<TreeNode>>& nodes) {
  for (auto& node : nodes) {
    node->lock_.unlock();
  }
}

#endif  // HASHSETS_TREE_MAP_FINE_GRAINED_H
