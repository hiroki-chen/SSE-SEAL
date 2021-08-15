/*
 Copyright (c) 2021 Haobin Chen

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef OBJECTS_H_
#define OBJECTS_H_

#include <map>
#include <string>
#include <vector>

#include <cereal/access.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/vector.hpp>
#include <oram/PathORAM.h>

/**
 * @brief The namsapce ODict defines all the objects needed for the access to the oblivious data structure.
 * 
 * It contains
 *  1. the class Node which is used to represent the node in the AVL tree data structure;
 *  2. the class Operation which is used to represent an access operation (WRITE / READ / INSERT / DELETE) for
 *     the access to the oblivious data structure. Note that the data is stored by its reference to the original
 *     one.
 * 
 *  Furthermore, in order to store the node in the oblivious ram, the cereal library is used for the serialization
 *  of the data, which allows all the class to be stored in std::string form.
 */
namespace ODict {
struct ChildrenPos {
    /* Intrusive serialization helper. */
    friend class cereal::access;
    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(id, pos_tag);
    }

public:
    int id;
    int pos_tag;

    ChildrenPos() = default;

    ChildrenPos(const int& id, const int& pos_tag);
};

struct Node {
    /* Intrusive serialization helper. */
    friend class cereal::access;
    template <typename Archive>
    void serialize(Archive& ar)
    {
        /* The structure childrenPos is serialized in a recursive manner. */
        ar(data, id, pos_tag, old_tag, key, left_height, right_height, childrenPos, left_id, right_id);
    }

public:
    std::string data; // The content of the data. It should be a VALID string!
    int id; // id is the address.
    int pos_tag;
    int old_tag = -1;

    std::string key;

    int left_height; // The height of the left sub-tree. Not sure if this is needed.
    int right_height; // The height of the right sub-tree.

    // An AVL Tree is a binary search tree.
    // Stores the children's position tag.
    ChildrenPos childrenPos[2];

    int left_id = 0;
    int right_id = 0;

    Node() = default;

    Node(const int& id, const int& pos_tag);

    Node(const Node& node);
};

struct TDAGNode : public Node {
    int parent_id = 0;
};

struct Operation {
public:
    int id; // Id of the data in the Oblivious data stucture.
    std::string data; // Data to be read / written.
    OramAccessOp op; // Operation type.

    Operation(const int& id, const std::string& data, OramAccessOp op);
};
}

namespace SEAL {
struct Document {
    friend class cereal::access;
    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(id, keywords);
    }

public:
    unsigned int id;
    std::vector<std::string> keywords;

    Document() = default;

    Document(const unsigned int& id, const std::vector<std::string>& keywords);
};
} // namespace SEAL

namespace Range {
/**
 * @brief Nodes used for the implementation of LOGRITHMIC-SRC-i (where SRC stands for Single Range Cover).
 * 
 * Single Range Covers allows us to always find a node N such that is completely covers the desired range
 * (none of its sub-tree covers the range), but with the support of TDAG, i.e., 
 * some extra nodes must be added to the tree.
 * 
 * T1 stores some metadata while T2 stores the actual encrypted tuples.
 * 
 * There are two types of trees in the LOGRITHMIC-SRC-i approach.
 * The first one is an auxilliary tree which will produce false positives and is used to guide the search on T2.
 * 
 * In addition, the RANGE-SRC-SE-(alpha,x) requires to build the secret index on T2 on the server side, using the
 * obilivious data structure (i.e., the oblivious dictionary.) to look up the intended documents.
 * 
 * It works as follows.
 *      1. The client wishes to search all the documents that contain keywords in range [lower, upper];
 *      2. It then selects the pairs that satisfy the query, 
 *         and creates a new, single query range on the document subscripts, by merging the qualifying ranges.
 *      3. It issues these merged ranges to T2, creating the tokens for nodes in T2.
 *      4. Since T2 is build in the form of oblivious data structure, the access pattern will be protected.
 */
struct Node {
    friend class cereal::access;
    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(documents, range_cover, id, left, right, parent);
    }

public:
    /* For which document ids the node stands. 
       This member only applies when the node is a leaf node.
       For internal node, the document ids can be retrived
       by its child nodes.
    */
    std::vector<unsigned int> documents;

    /* The range the node covers. E.g. [0,9] means that the child nodes cover 0 ~ 9. 
       The keyword for each node is the range covered by each node.
       Binary search is done by comparing the range of each node.

       Only support numeric search.
    */
    std::pair<int, int> range_cover;

    unsigned int id;

    static unsigned int counter;

    Node* left;
    Node* right;
    // Parent is the internal node (marked as black)
    Node* parent;

    Node() = default;

    /**
     * @brief Construct a new Node object.
     * 
     * The tree is built in a recursive manner, so the document ids are set after the child nodes are built.
     * 
     * @param lhs          The left range.
     * @param rhs          The right range.
     */
    Node(const int& lhs, const int& rhs);
};
} // namespace Range

#endif