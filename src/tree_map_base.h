
#ifndef UNTITLED_TREE_MAP_BASE_H
#define UNTITLED_TREE_MAP_BASE_H
#include <cstddef>
#include <vector>
#include <optional>

using ByteArray = std::vector<std::byte>;

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
