#include "src/tree_map_fine_grained.h"
#include <algorithm>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

void TreeMapFineGrained::left_rotate(std::shared_ptr<TreeNode> x) {
    if (x == nullptr || x->right_ == nullptr) return;

    std::shared_ptr<TreeNode> y = x->right_;
    std::shared_ptr<TreeNode> parent = x->get_parent();

    // Perform rotation (pointer manipulation only)
    x->right_ = y->left_;
    if (y->left_ != nullptr) {
        y->left_->parent_ = x;
    }

    y->parent_ = x->parent_; // Assign weak_ptr from weak_ptr

    if (parent == nullptr) {
        std::scoped_lock root_lock(root_lock_); // Lock just for root update
        root = y;
    } else {
        if (x == parent->left_) {
            parent->left_ = y;
        } else {
            parent->right_ = y;
        }
    }
    y->left_ = x;
    x->parent_ = y;
}

void TreeMapFineGrained::right_rotate(std::shared_ptr<TreeNode> y) {
    if (y == nullptr || y->left_ == nullptr) return;

    std::shared_ptr<TreeNode> x = y->left_;
    std::shared_ptr<TreeNode> parent = y->get_parent();

    // Perform rotation (pointer manipulation only)
    y->left_ = x->right_;
    if (x->right_ != nullptr) {
        x->right_->parent_ = y;
    }

    x->parent_ = y->parent_;

    if (parent == nullptr) {
        std::scoped_lock root_lock(root_lock_); // Lock just for root update
        root = x;
    } else {
        if (y == parent->left_) {
            parent->left_ = x;
        } else {
            parent->right_ = x;
        }
    }
    x->right_ = y;
    y->parent_ = x;
}

void TreeMapFineGrained::rbt_insert_fixup(std::shared_ptr<TreeNode> z) {
    std::shared_ptr<TreeNode> current = std::move(z);

    while (current != root) {
        // --- 1. Optimistic (Unlocked) Reads ---
        // These are racy, but we validate after locking.
        std::shared_ptr<TreeNode> parent = current->get_parent();
        if (parent == nullptr || parent->color_ != RED) {
            break; // Parent is BLACK or root, all good
        }

        std::shared_ptr<TreeNode> grandparent = parent->get_parent();
        if (grandparent == nullptr) {
            break; // Parent is root, loop will stop
        }

        std::shared_ptr<TreeNode> uncle = (parent == grandparent->left_) ?
                                        grandparent->right_ : grandparent->left_;

        // --- 2. Case 1: Uncle is RED (Recoloring) ---
        if (uncle != nullptr && uncle->color_ == RED) {
            std::vector<std::shared_ptr<TreeNode>> nodes_to_lock = {parent, grandparent, uncle};
            // lock_nodes(nodes_to_lock);

            // --- State Validation Block ---
            if (parent->get_parent() != grandparent || parent->color_ != RED ||
                uncle->get_parent() != grandparent || uncle->color_ != RED)
            {
                // unlock_nodes(nodes_to_lock);
                continue; // State changed, restart loop from 'current'
            }

            parent->color_ = BLACK;
            uncle->color_ = BLACK;
            grandparent->color_ = RED;

            // unlock_nodes(nodes_to_lock);
            current = grandparent; // Move up
            continue; // Continue loop from new 'current'
        }

        // --- 3. Case 2/3: Uncle is BLACK (Rotations) ---
        if (parent == grandparent->left_) {
            // Case 2: Triangle (Left-Right)
            if (current == parent->right_) {
                std::vector<std::shared_ptr<TreeNode>> nodes_to_lock = {parent, current, grandparent};
                if(current->left_ != nullptr) nodes_to_lock.push_back(current->left_);

                // lock_nodes(nodes_to_lock);
                // (Re-check state here...)
                if(parent->get_parent() != grandparent || current->get_parent() != parent) {
                    // unlock_nodes(nodes_to_lock);
                    continue; // State changed
                }

                left_rotate(parent);

                // unlock_nodes(nodes_to_lock);

                current = parent; // 'current' is now the old parent
                parent = current->get_parent();
                if (parent == nullptr) continue; // Restart loop
            }

            // Case 3: Line (Left-Left)
            std::vector<std::shared_ptr<TreeNode>> nodes_to_lock = {parent, grandparent};
            if(grandparent->get_parent() != nullptr) nodes_to_lock.push_back(grandparent->get_parent());
            if(parent->right_ != nullptr) nodes_to_lock.push_back(parent->right_);

            // lock_nodes(nodes_to_lock);

            if (parent->get_parent() != grandparent || parent->color_ != RED) {
                // unlock_nodes(nodes_to_lock);
                continue;
            }

            parent->color_ = BLACK;
            grandparent->color_ = RED;
            right_rotate(grandparent);

            // unlock_nodes(nodes_to_lock);

        } else {
            // Case 2: Triangle (Right-Left)
            if (current == parent->left_) {
                std::vector<std::shared_ptr<TreeNode>> nodes_to_lock = {parent, current, grandparent};
                if(current->right_ != nullptr) nodes_to_lock.push_back(current->right_);

                // lock_nodes(nodes_to_lock);
                // (Re-check state here...)
                if(parent->get_parent() != grandparent || current->get_parent() != parent) {
                    // unlock_nodes(nodes_to_lock);
                    continue; // State changed
                }

                right_rotate(parent);

                // unlock_nodes(nodes_to_lock);

                current = parent;
                parent = current->get_parent();
                if (parent == nullptr) continue; // Restart loop
            }

            // Case 3: Line (Right-Right)
            std::vector<std::shared_ptr<TreeNode>> nodes_to_lock = {parent, grandparent};
            if(grandparent->get_parent() != nullptr) nodes_to_lock.push_back(grandparent->get_parent());
            if(parent->left_ != nullptr) nodes_to_lock.push_back(parent->left_);

            // lock_nodes(nodes_to_lock);

            if (parent->get_parent() != grandparent || parent->color_ != RED) {
                // unlock_nodes(nodes_to_lock);
                continue;
            }

            parent->color_ = BLACK;
            grandparent->color_ = RED;
            left_rotate(grandparent);

            // unlock_nodes(nodes_to_lock);
        }
        break; // Rotations always terminate the loop
    }

    root_lock_.lock();
    if (root != nullptr) {
        root->color_ = BLACK;
    }
    root_lock_.unlock();
}

std::optional<ByteArray> TreeMapFineGrained::get(const ByteArray& key) {
    root_lock_.lock_shared();
    std::shared_ptr<TreeNode> current = root;

    if (current == nullptr) {
        root_lock_.unlock_shared();
        return std::nullopt;
    }

    // HOH: Lock first node before unlocking root
    current->lock_.lock_shared();
    root_lock_.unlock_shared();

    while (current != nullptr) {
        if (is_equal(key, current->key_)) {
            ByteArray value_copy = current->value_; // Copy value
            current->lock_.unlock_shared();
            return std::optional{std::move(value_copy)};
        }

        // Determine next node
        std::shared_ptr<TreeNode> next = is_less(key, current->key_) ?
                                       current->left_ : current->right_;

        if (next == nullptr) {
            current->lock_.unlock_shared();
            break; // Not found
        }

        // HOH: Lock next node before unlocking current node
        next->lock_.lock_shared();
        current->lock_.unlock_shared();
        current = next;
    }

    return std::nullopt; // Not found
}

void TreeMapFineGrained::put(ByteArray key, ByteArray value) {
    auto z = std::make_shared<TreeNode>(key, value);

    // Retry loop for concurrent insertions
    while (true) {
        root_lock_.lock_shared();
        std::shared_ptr<TreeNode> x = root; // Current node
        std::shared_ptr<TreeNode> y = nullptr; // Parent node

        if (x == nullptr) {
            root_lock_.unlock_shared();
            root_lock_.lock(); // Upgrade to exclusive lock
            if (root == nullptr) { // Re-check after lock
                root = z;
                root->color_ = BLACK;
                root_lock_.unlock();
                return; // Success
            }
            root_lock_.unlock();
            continue; // Lost race, retry
        }

        // Lock first node before unlocking root
        x->lock_.lock_shared();
        root_lock_.unlock_shared();

        while (x != nullptr) {
            y = x; // y is the parent, x is current

            if (is_equal(key, x->key_)) {
                // Found: upgrade to write lock for overwrite
                x->lock_.unlock_shared();
                std::scoped_lock<std::shared_mutex> write_lock(x->lock_);
                x->value_ = std::move(value);
                return; // Success
            }

            std::shared_ptr<TreeNode> next = is_less(key, x->key_) ? x->left_ : x->right_;

            if (next == nullptr) {
                break; // Found insertion point, y is parent
            }

            // HOH: Lock next, unlock current
            next->lock_.lock_shared();
            x->lock_.unlock_shared();
            x = next;
        }

        // y is the parent, x is null. y is still locked (shared).
        y->lock_.unlock_shared();

        // Upgrade to exclusive lock on parent (y)
        std::scoped_lock<std::shared_mutex> parent_write_lock(y->lock_);

        // Re-check for race conditions after upgrading lock
        bool should_go_left = is_less(z->key_, y->key_);
        std::shared_ptr<TreeNode> expected_child = should_go_left ? y->left_ : y->right_;

        if (expected_child != nullptr) {
            continue; // Lost race, retry
        }

        // Check for overwrite on y (if key == y->key_)
        if (is_equal(key, y->key_)) {
            y->value_ = std::move(value);
            return; // Success
        }

        // Perform insertion
        z->parent_ = y; // Assign shared_ptr to weak_ptr
        if (should_go_left) {
            y->left_ = z;
        } else {
            y->right_ = z;
        }

        // We hold the lock on 'y' (the parent) while we start the fixup.
        // The fixup function will handle its own locking from here.
        rbt_insert_fixup(z);
        return; // Success
    }
}
