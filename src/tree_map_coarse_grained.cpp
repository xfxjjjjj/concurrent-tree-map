#include "src/tree_map_coarse_grained.h"

// Adds |key| to the hash map.
void TreeMapCoarseGrained::put(ByteArray key, ByteArray value) {
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
std::optional<ByteArray> TreeMapCoarseGrained::get(const ByteArray& key) {
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

