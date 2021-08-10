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

#include <oram/PathORAM.h>
#include <cereal/access.hpp>

namespace ODict {
struct ChildrenPos {
    friend class cereal::access;
    template<typename Archive>
    void serialize(Archive& ar) {
        ar(id, pos_tag);
    }

public:
    int id;
    int pos_tag;

    ChildrenPos() = default;

    ChildrenPos(const int& id, const int& pos_tag);
};

struct Node {
    friend class cereal::access;
    template<typename Archive>
    void serialize(Archive& ar) {
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

struct Operation {
public:
    int id; // Id of the data in the Oblivious data stucture.
    std::string data; // Data to be read / written.
    OramAccessOp op; // Operation type.

    Operation(const int& id, const std::string& data, OramAccessOp op);
};
}

#endif