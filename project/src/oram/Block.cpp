#include <oram/Block.h>

#include <iostream>
#include <sstream>
#include <string>
Block::Block(const Block& block)
{
    index = block.index;
    leaf_id = block.leaf_id;
    data = block.data;
}

Block::Block(
    const int& leaf_id,
    const int& index,
    const std::string& data)
    : leaf_id(leaf_id)
    , index(index)
    , data(data)
{
}

Block::Block()
    : leaf_id(-1)
    , index(-1)
    , data("")
{
}

Block::~Block()
{
}

void Block::printBlock()
{
    std::string data_holder;
    std::for_each(data.begin(), data.end(), [&data_holder](const char& c) {
        data_holder.push_back(c);
    });

    std::cout << "index: " << std::to_string(index)
              << " leaf id: " << std::to_string(leaf_id) << " data: " << data_holder
              << std::endl;
}