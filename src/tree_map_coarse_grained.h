#ifndef UNTITLED_COARSE_GRAINED_TREE_MAP_H
#define UNTITLED_COARSE_GRAINED_TREE_MAP_H

#include <src/tree_map_base.h>

#include <mutex>
#include <utility>

#include "benchmark.h"

class TreeMapCoarseGrained : public TreeMapBase {
 public:
  // ... constructor and members ...
  void put(ByteArray key, ByteArray value) override;  // Declaration only
  std::optional<ByteArray> get(
      const ByteArray& key) override;  // Declaration only
 private:
  std::mutex mutex_;
  std::shared_ptr<TreeNode> root{nullptr};
  void left_rotate(std::shared_ptr<TreeNode> x);
  void right_rotate(std::shared_ptr<TreeNode> y);
  void rbt_insert_fixup(std::shared_ptr<TreeNode> cur);
};

#endif  // UNTITLED_COARSE_GRAINED_TREE_MAP_H
