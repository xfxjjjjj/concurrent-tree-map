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

        if (uncle != nullptr && uncle->color_ == RED) {
            std::vector<std::shared_ptr<TreeNode>> nodes_to_lock = {parent, grandparent, uncle};
            lock_nodes(nodes_to_lock);

            // --- State Validation Block ---
            if (parent->get_parent() != grandparent || parent->color_ != RED ||
                uncle->get_parent() != grandparent || uncle->color_ != RED)
            {
                unlock_nodes(nodes_to_lock);
                continue; // State changed, restart loop from 'current'
            }

            parent->color_ = BLACK;
            uncle->color_ = BLACK;
            grandparent->color_ = RED;

            unlock_nodes(nodes_to_lock);
            current = grandparent; // Move up
            continue; // Continue loop from new 'current'
        }

        if (parent == grandparent->left_) {
            // Triangle (Left-Right)
            if (current == parent->right_) {
                std::vector<std::shared_ptr<TreeNode>> nodes_to_lock = {parent, current, grandparent};
                if(current->left_ != nullptr) nodes_to_lock.push_back(current->left_);

                lock_nodes(nodes_to_lock);
                if(parent->get_parent() != grandparent || current->get_parent() != parent) {
                    unlock_nodes(nodes_to_lock);
                    continue; // State changed
                }

                left_rotate(parent);

                unlock_nodes(nodes_to_lock);

                current = parent; // 'current' is now the old parent
                parent = current->get_parent();
                if (parent == nullptr) continue; // Restart loop
            }

            // Line (Left-Left)
            std::vector<std::shared_ptr<TreeNode>> nodes_to_lock = {parent, grandparent};
            if(grandparent->get_parent() != nullptr) nodes_to_lock.push_back(grandparent->get_parent());
            if(parent->right_ != nullptr) nodes_to_lock.push_back(parent->right_);

            lock_nodes(nodes_to_lock);

            if (parent->get_parent() != grandparent || parent->color_ != RED) {
                unlock_nodes(nodes_to_lock);
                continue;
            }

            parent->color_ = BLACK;
            grandparent->color_ = RED;
            right_rotate(grandparent);

            unlock_nodes(nodes_to_lock);

        } else {
            // Triangle (Right-Left)
            if (current == parent->left_) {
                std::vector<std::shared_ptr<TreeNode>> nodes_to_lock = {parent, current, grandparent};
                if(current->right_ != nullptr) nodes_to_lock.push_back(current->right_);

                // lock_nodes(nodes_to_lock);
                lock_nodes(nodes_to_lock);
                if(parent->get_parent() != grandparent || current->get_parent() != parent) {
                    unlock_nodes(nodes_to_lock);
                    continue; // State changed
                }

                right_rotate(parent);

                unlock_nodes(nodes_to_lock);

                current = parent;
                parent = current->get_parent();
                if (parent == nullptr) continue; // Restart loop
            }

            // Line (Right-Right)
            std::vector<std::shared_ptr<TreeNode>> nodes_to_lock = {parent, grandparent};
            if(grandparent->get_parent() != nullptr) nodes_to_lock.push_back(grandparent->get_parent());
            if(parent->left_ != nullptr) nodes_to_lock.push_back(parent->left_);

            lock_nodes(nodes_to_lock);

            if (parent->get_parent() != grandparent || parent->color_ != RED) {
                unlock_nodes(nodes_to_lock);
                continue;
            }

            parent->color_ = BLACK;
            grandparent->color_ = RED;
            left_rotate(grandparent);

            unlock_nodes(nodes_to_lock);
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
        std::shared_ptr<TreeNode> x; // Current node
        std::shared_ptr<TreeNode> y = nullptr; // Parent node

        // --- 1. Handle Empty Tree & Get Root Safely ---
        {
            std::cout << "[TID " << std::this_thread::get_id() << " ...waiting for root lock" << std::endl;
            std::scoped_lock<std::shared_mutex> root_guard(root_lock_);
            std::cout << "[TID " << std::this_thread::get_id() << " ...acquired for root lock" << std::endl;

            if (root == nullptr) {
                root = z;
                root->color_ = BLACK;
                return; // Success
            }
            x = root;
            x->lock_.lock_shared(); // Lock the root before releasing root_lock_
            std::cout << "[TID " << std::this_thread::get_id() << " ...released for root lock" << std::endl;

        } // root_lock_ is released

        // --- 2. One-Lock-At-A-Time HOH Traversal ---
        while (x != nullptr) {
            // 'x' is locked (shared) at the start of this loop

            y = x; // 'y' is now the potential parent

            if (is_equal(key, x->key_)) {
                // Found: upgrade to write lock for overwrite
                x->lock_.unlock_shared();
                std::cout << "[TID " << std::this_thread::get_id() << " ...waiting  " << static_cast<void *> (x.get()) << std::endl;
                std::scoped_lock<std::shared_mutex> write_lock(x->lock_);
                std::cout << "[TID " << std::this_thread::get_id() << " ...acquired  " << static_cast<void *> (x.get()) << std::endl;

                x->value_ = std::move(value);
                return; // Success
            }

            // 1. Safely copy the next shared_ptr (increments ref count)
            std::shared_ptr<TreeNode> next = is_less(key, x->key_) ? x->left_ : x->right_;

            // 2. Release the current node's lock
            x->lock_.unlock_shared();
            std::cout << "[TID " << std::this_thread::get_id() << " ...released  " << static_cast<void *> (x.get()) << std::endl;


            // 3. Move to the next node
            x = next;

            if (x != nullptr) {
                // 4. Lock the *next* node for the next iteration
                std::cout << "[TID " << std::this_thread::get_id() << " ...waiting  " << static_cast<void *> (x.get()) << std::endl;
                x->lock_.lock_shared();
                std::cout << "[TID " << std::this_thread::get_id() << " ...acquired  " << static_cast<void *> (x.get()) << std::endl;

            }
            // Loop continues. 'y' (the parent) is now UNLOCKED.
        }

        // --- 3. Insertion Phase ---
        // At this point, x is null, and y is the parent.
        // CRITICAL: 'y' is UNLOCKED. We must lock it exclusively.

        // If y is null here, it means the root was locked and then
        // immediately found to be the parent, which shouldn't happen
        // with the new root check. But we check anyway.
        if (y == nullptr) {
             // This case should no longer be possible.
             continue; // Retry
        }

        std::scoped_lock<std::shared_mutex> parent_write_lock(y->lock_);

        // Re-check for race conditions
        bool should_go_left = is_less(z->key_, y->key_);
        std::shared_ptr<TreeNode> expected_child = should_go_left ? y->left_ : y->right_;

        if (expected_child != nullptr) {
            // We lost a race. Another thread inserted here.
            continue; // Restart the entire 'put' operation
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
        y->lock_.unlock();
        std::cout << "[TID " << std::this_thread::get_id() << " ...released  " << static_cast<void *> (y.get()) << std::endl;

        // --- 4. Fix-up ---
        rbt_insert_fixup(z);
        return; // Success
    }
}
