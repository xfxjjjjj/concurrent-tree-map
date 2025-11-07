#ifndef HASHSETS_TREE_MAP_FINE_GRAINED_H
#define HASHSETS_TREE_MAP_FINE_GRAINED_H
#include <src/tree_map_base.h>

#include <mutex>
#include <iostream>
#include <thread>       // For logging
#include <sstream>      // For logging

static std::mutex g_log_mutex;

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
  std::sort(nodes.begin(), nodes.end(), [](const auto& a, const auto& b) {
      return a.get() < b.get();
  });
  nodes.erase(std::remove(nodes.begin(), nodes.end(), nullptr), nodes.end());
  nodes.erase(std::unique(nodes.begin(), nodes.end()), nodes.end());

  {
    std::scoped_lock<std::mutex> log_lock(g_log_mutex);
    std::cout << "[TID " << std::this_thread::get_id() << "] Trying to lock " << nodes.size() << " nodes: ";
    for(auto& n : nodes) std::cout << static_cast<void *>(n.get()) << " ";
    std::cout << std::endl;
  }

  for (auto& node : nodes) {
    {
      std::scoped_lock<std::mutex> log_lock(g_log_mutex);
      std::cout << "[TID " << std::this_thread::get_id() << "] ...waiting for " << static_cast<void *>(node.get()) << std::endl;
    }
    node->lock_.lock(); // Acquire exclusive lock
    {
      std::scoped_lock<std::mutex> log_lock(g_log_mutex);
      std::cout << "[TID " << std::this_thread::get_id() << "] ...acquired " << static_cast<void *>(node.get()) << std::endl;
    }
  }
}

// Helper function to unlock multiple nodes
inline void unlock_nodes(std::vector<std::shared_ptr<TreeNode>>& nodes) {
  for (auto& node : nodes) {
    node->lock_.unlock();
    std::cout << "[TID " << std::this_thread::get_id() << "] ...released " << static_cast<void *>(node.get()) << std::endl;
  }
}

#endif  // HASHSETS_TREE_MAP_FINE_GRAINED_H
