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

class TreeMapCoarseGrained final : TreeMapBase {
public:

    explicit TreeMapCoarseGrained() = default;

    // Adds |key| to the hash map.
    void put(ByteArray key, ByteArray value) override {
        auto newNode = TreeNode(std::move(key), std::move(value));
        std::shared_ptr<TreeNode> y = nullptr;

        std::scoped_lock<std::mutex> lock(mutex_);
        std::shared_ptr<TreeNode> x = root;

        while (x != nullptr) {
            y = x;
            if (newNode.key_ < x->key_)
                x = x->left_;
            else
                x = x->right_;
        }

        newNode.parent_ = y;
        if (y == nullptr)
            root = std::make_shared<TreeNode>(newNode);
        else if (newNode.key_ < y->key_)
            y->left_ = std::make_shared<TreeNode>(newNode);
        else if (newNode.key_ > y->key_)
            y->right_ = std::make_shared<TreeNode>(newNode);
        else
            y->value_ = std::move(value);


    }


    // Returns true if |key| is present in the hash set, and false otherwise.
    std::optional<ByteArray> get(const ByteArray& key) override {
        std::scoped_lock<std::mutex> lock(mutex_);
        std::shared_ptr<TreeNode> x = root;


        while (x != nullptr) {
            if (x->key_ == key) {
                return std::optional {x->value_}; // Key found!
            }

            if (is_less(key, x->key_)) {
                x = x->left_;
            } else {
                x = x->right_;
            }
        }

        return std::nullopt;
    }

private:
    std::mutex mutex_;
    std::shared_ptr<TreeNode> root {nullptr};
};

#endif //UNTITLED_COARSE_GRAINED_TREE_MAP_H
