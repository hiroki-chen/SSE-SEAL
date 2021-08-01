#include <Objects.h>

ODict::Operation::Operation(const int& id, char* data, OramAccessOp op)
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

ODict::ChildrenPos::ChildrenPos(const int& id, const int& pos_tag)
    : id(id)
    , pos_tag(pos_tag)
{
}