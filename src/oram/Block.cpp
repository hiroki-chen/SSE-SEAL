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
    if (index != -1 && leaf_id != -1) {
        std::cout << "index: " << std::to_string(index)
                  << " leaf id: " << std::to_string(leaf_id) << " data: " << data
                  << std::endl;
    }
}