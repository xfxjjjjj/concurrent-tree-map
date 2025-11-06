
#ifndef UNTITLED_TREE_MAP_BASE_H
#define UNTITLED_TREE_MAP_BASE_H
#include <cstddef>
#include <vector>
#include <optional>
#include <mutex>
#include <utility>

using ByteArray = std::vector<std::byte>;

enum Color {RED, BLACK};

struct TreeNode {
    Color color_ = RED;
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

class TreeMapBase {
public:
    virtual ~TreeMapBase();

    // Adds |key| to the hash set. Returns true if |elem| was absent, and false
    // otherwise.
    virtual void put(ByteArray key, ByteArray value) = 0;


    // Returns true if |key| is present in the hash set, and false otherwise.
    virtual std::optional<ByteArray> get(const ByteArray& key) = 0;

};

#endif //UNTITLED_TREE_MAP_BASE_H
