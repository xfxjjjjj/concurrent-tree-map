#include "src/tree_map_coarse_grained.h"

void TreeMapCoarseGrained::left_rotate(std::shared_ptr<TreeNode> x) {
    if (x == nullptr || x->right_ == nullptr)
        return;

    std::shared_ptr<TreeNode> y = x->right_;
    x->right_ = y->left_;
    if (y->left_ != nullptr)
        y->left_->parent_ = x;  // weak_ptr assignment

    std::shared_ptr<TreeNode> parent = x->get_parent();
    y->parent_ = parent;  // weak_ptr assignment

    if (parent == nullptr)
        root = y;
    else if (x == parent->left_)
        parent->left_ = y;
    else
        parent->right_ = y;

    y->left_ = x;
    x->parent_ = y;  // weak_ptr assignment
}

void TreeMapCoarseGrained::right_rotate(std::shared_ptr<TreeNode> y) {
    if (y == nullptr || y->left_ == nullptr)
        return;

    std::shared_ptr<TreeNode> x = y->left_;
    y->left_ = x->right_;
    if (x->right_ != nullptr)
        x->right_->parent_ = y;  // weak_ptr assignment

    std::shared_ptr<TreeNode> parent = y->get_parent();
    x->parent_ = parent;  // weak_ptr assignment

    if (parent == nullptr)
        root = x;
    else if (y == parent->left_)
        parent->left_ = x;
    else
        parent->right_ = x;

    x->right_ = y;
    y->parent_ = x;  // weak_ptr assignment
}

void TreeMapCoarseGrained::rbt_insert_fixup(std::shared_ptr<TreeNode> cur) {
    while (cur != root) {
        std::shared_ptr<TreeNode> parent = cur->get_parent();
        if (parent == nullptr || parent->color_ != RED)
            break;

        std::shared_ptr<TreeNode> grandparent = parent->get_parent();
        if (grandparent == nullptr)
            break;

        if (parent == grandparent->left_) {
            std::shared_ptr<TreeNode> uncle = grandparent->right_;

            if (uncle != nullptr && uncle->color_ == RED) {
                // Case 1: Uncle is RED - recoloring
                parent->color_ = BLACK;
                uncle->color_ = BLACK;
                grandparent->color_ = RED;
                cur = grandparent;
            } else {
                // Case 2/3: Uncle is BLACK - rotations needed
                if (cur == parent->right_) {
                    // Case 2: Left-right case
                    cur = parent;
                    left_rotate(cur);
                    parent = cur->get_parent(); // Update parent after rotation
                    if (parent == nullptr) break;
                }

                // Case 3: Left-left case
                parent->color_ = BLACK;
                grandparent->color_ = RED;
                right_rotate(grandparent);
                break;
            }
        } else {
            std::shared_ptr<TreeNode> uncle = grandparent->left_;

            if (uncle != nullptr && uncle->color_ == RED) {
                // Case 1: Uncle is RED - recoloring
                parent->color_ = BLACK;
                uncle->color_ = BLACK;
                grandparent->color_ = RED;
                cur = grandparent;
            } else {
                // Case 2/3: Uncle is BLACK - rotations needed
                if (cur == parent->left_) {
                    // Case 2: Right-left case
                    cur = parent;
                    right_rotate(cur);
                    parent = cur->get_parent(); // Update parent after rotation
                    if (parent == nullptr) break;
                }

                // Case 3: Right-right case
                parent->color_ = BLACK;
                grandparent->color_ = RED;
                left_rotate(grandparent);
                break;
            }
        }
    }

    // Final Rule: Root must be BLACK.
    if (root != nullptr) {
        root->color_ = BLACK;
    }
}

void TreeMapCoarseGrained::put(ByteArray key, ByteArray value) {
    auto newNode = std::make_shared<TreeNode>(std::move(key), std::move(value));
    std::shared_ptr<TreeNode> y = nullptr;

    std::scoped_lock<std::mutex> lock(mutex_);
    std::shared_ptr<TreeNode> x = root;

    // Find insertion point
    while (x != nullptr) {
        y = x;
        if (newNode->key_ < x->key_)
            x = x->left_;
        else if (newNode->key_ > x->key_)
            x = x->right_;
        else {
            // Key already exists - update value
            x->value_ = std::move(value);
            return;
        }
    }

    // Insert new node
    newNode->parent_ = y;  // weak_ptr assignment

    if (y == nullptr) {
        root = newNode;
    } else if (newNode->key_ < y->key_) {
        y->left_ = newNode;
    } else {
        y->right_ = newNode;
    }

    rbt_insert_fixup(newNode);
}

std::optional<ByteArray> TreeMapCoarseGrained::get(const ByteArray& key) {
    std::scoped_lock<std::mutex> lock(mutex_);
    std::shared_ptr<TreeNode> x = root;

    while (x != nullptr) {
        if (x->key_ == key) {
            return std::optional{x->value_};
        }

        if (is_less(key, x->key_)) {
            x = x->left_;
        } else {
            x = x->right_;
        }
    }

    return std::nullopt;
}
