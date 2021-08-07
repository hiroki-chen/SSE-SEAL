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