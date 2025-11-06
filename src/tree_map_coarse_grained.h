#ifndef UNTITLED_COARSE_GRAINED_TREE_MAP_H
#define UNTITLED_COARSE_GRAINED_TREE_MAP_H

#include <mutex>
#include <utility>
#include <src/tree_map_base.h>
#include "benchmark.h"

struct TreeNode {
    ByteArray key_;
    ByteArray value_;
    std::shared_ptr<TreeNode> left_;
    std::shared_ptr<TreeNode> right_;
    std::shared_ptr<TreeNode> parent_;

    explicit TreeNode(ByteArray key, ByteArray value)
        : key_ {std::move(key)}, value_ {std::move(value)} {}
};

inline bool is_less(const ByteArray& a, const ByteArray& b) {
    return std::ranges::lexicographical_compare(a, b);
}

class TreeMapCoarseGrained : public TreeMapBase {
public:
    // ... constructor and members ...
    void put(ByteArray key, ByteArray value) override; // Declaration only
    std::optional<ByteArray> get(const ByteArray& key) override; // Declaration only
private:
    std::mutex mutex_;
    std::shared_ptr<TreeNode> root {nullptr};
};

#endif //UNTITLED_COARSE_GRAINED_TREE_MAP_H
