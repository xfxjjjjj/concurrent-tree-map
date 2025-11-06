#include "src/tree_map_coarse_grained.h"

void TreeMapCoarseGrained::left_rotate(std::shared_ptr<TreeNode> x) {
    if (x == nullptr || x->right_ == nullptr)
        return;

    std::shared_ptr<TreeNode> y = x->right_;
    x->right_ = y->left_;
    if (y->left_ != nullptr)
        y->left_->parent_ = x;
    y->parent_ = x->parent_;
    if (x->parent_ == nullptr)
        root = y;
    else if (x == x->parent_->left_)
        x->parent_->left_ = y;
    else
        x->parent_->right_ = y;
    y->left_ = x;
    x->parent_ = y;
}

void TreeMapCoarseGrained::right_rotate(std::shared_ptr<TreeNode> y) {
    std::shared_ptr<TreeNode> x = y->left_;
    y->left_ = x->right_;
    if (x->right_ != nullptr)
        x->right_->parent_ = y;
    x->parent_ = y->parent_;
    if (y->parent_ == nullptr)
        root = x;
    else if (y == y->parent_->left_)
        y->parent_->left_ = x;
    else
        y->parent_->right_ = x;
    x->right_ = y;
    y->parent_ = x;
}

void TreeMapCoarseGrained::rbt_insert_fixup(std::shared_ptr<TreeNode> cur) {
    while (cur != root && cur->parent_->color_ == RED) {
        if (cur->parent_ == cur->parent_->parent_->left_) {
            if (std::shared_ptr<TreeNode> uncle = cur->parent_->parent_->right_;
                uncle != nullptr && uncle->color_ == RED) {
                cur->parent_->color_ = BLACK;
                uncle->color_ = BLACK;
                cur->parent_->parent_->color_ = RED;
                cur = cur->parent_->parent_;
            } else {
                if (cur == cur->parent_->right_) {
                    cur = cur->parent_;
                    left_rotate(cur);
                }
                cur->parent_->color_ = BLACK;
                cur->parent_->parent_->color_ = RED;
                right_rotate(cur->parent_->parent_);
            }
        } else {
            if (std::shared_ptr<TreeNode> uncle = cur->parent_->parent_->left_;
                uncle != nullptr && uncle->color_ == RED) {
                cur->parent_->color_ = BLACK;
                uncle->color_ = BLACK;
                cur->parent_->parent_->color_ = RED;
                cur = cur->parent_->parent_;
            } else {
                if (cur == cur->parent_->left_) {
                    cur = cur->parent_;
                    right_rotate(cur);
                }
                cur->parent_->color_ = BLACK;
                cur->parent_->parent_->color_ = RED;
                left_rotate(cur->parent_->parent_);
            }
        }
    }
    root->color_ = BLACK;
}

// Adds |key| to the hash map.
void TreeMapCoarseGrained::put(ByteArray key, ByteArray value) {
    auto newNode = std::make_shared<TreeNode>(std::move(key), std::move(value));
    std::shared_ptr<TreeNode> y = nullptr;

    std::scoped_lock<std::mutex> lock(mutex_);
    std::shared_ptr<TreeNode> x = root;

    while (x != nullptr) {
        y = x;
        if (newNode->key_ < x->key_)
            x = x->left_;
        else
            x = x->right_;
    }

    newNode->parent_ = y;
    if (y == nullptr)
        root = newNode;
    else if (newNode->key_ < y->key_)
        y->left_ = newNode;
    else if (newNode->key_ > y->key_)
        y->right_ = newNode;
    else
        y->value_ = std::move(value);

    rbt_insert_fixup(newNode);

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

