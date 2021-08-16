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

#include <client/Objects.h>

ODict::Operation::Operation(const int& id, const std::string& data, OramAccessOp op)
    : id(id)
    , data(data)
    , op(op)
{
}

ODict::Node::Node(const int& id, const int& pos_tag)
    : id(id)
    , pos_tag(pos_tag)
{
}

ODict::Node::Node(const ODict::Node& node)
{
    data = node.data;
    id = node.id;
    pos_tag = node.pos_tag;
    old_tag = node.old_tag;
    key = node.key;
    left_height = node.left_height;
    right_height = node.right_height;

    childrenPos[0] = node.childrenPos[0];
    childrenPos[1] = node.childrenPos[1];
    left_id = node.left_id;
    right_id = node.right_id;
}

ODict::ChildrenPos::ChildrenPos(const int& id, const int& pos_tag)
    : id(id)
    , pos_tag(pos_tag)
{
}

SEAL::Document::Document(const unsigned int& id, const std::vector<std::string>& keywords)
    : id(id)
    , keywords(keywords.begin() + 1, keywords.end())
{
}

Range::Node::Node(
    const int& lhs, const int& rhs,
    const std::map<int, std::vector<unsigned int>>& kwd_doc_pairs)
    : kwd_doc_pairs(kwd_doc_pairs.begin(), kwd_doc_pairs.end())
    , range_cover(std::make_pair(lhs, rhs))
    , id(counter++)
    , left(nullptr)
    , right(nullptr)
    , parent(nullptr)

{
}